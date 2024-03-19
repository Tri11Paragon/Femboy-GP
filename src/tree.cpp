/*
 *  Copyright (C) 2024  Brett Terpstra
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <lilfbtf/tree.h>
#include <stack>

namespace fb
{
    
    func_t::func_t(blt::size_t argc, const func_t_call_t& func, type_id output_type, function_id function_type):
            argc_(argc), type(output_type), function(function_type), func(func)
    {}
    
    tree_t::tree_t(type_engine_t& types): alloc(), types(types)
    {}
    
    tree_t tree_t::make_tree(detail::tree_construction_info_t tree_info,
                             blt::size_t min_depth, blt::size_t max_depth, std::optional<type_id> starting_type)
    {
        using detail::node_t;
        tree_t tree(tree_info.types);
        {
            if (starting_type)
                tree.root = allocate_non_terminal({tree, tree_info}, starting_type.value());
            else
            {
                auto& non_terminals = tree_info.types.get_all_non_terminals();
                auto selection = non_terminals[tree_info.engine.random_long(0, non_terminals.size() - 1)];
                func_t func(tree_info.types.get_function_argc(selection.second), tree_info.types.get_function(selection.second), selection.first,
                            selection.second);
                if (const auto& func_init = tree_info.types.get_function_initializer(selection.second))
                    func_init.value()(func);
                tree.root = tree.alloc.template emplace<node_t>(func, tree.alloc);
            }
        }
        
        switch (tree_info.tree_type)
        {
            case tree_init_t::GROW:
                grow({tree, tree_info}, min_depth, max_depth);
                break;
            case tree_init_t::FULL:
                full({tree, tree_info}, tree_info.engine.random_long(min_depth, max_depth));
                break;
            case tree_init_t::RAMPED_HALF_HALF:
                break;
            case tree_init_t::BRETT_HALF_HALF:
                break;
        }
        
        return tree;
    }
    
    std::pair<blt::unsafe::any_t, type_id> tree_t::evaluate()
    {
        using detail::node_t;
        std::stack<node_t*> nodes;
        std::stack<node_t*> node_stack;
        
        nodes.push(root);
        
        // create the correct ordering for the node evaluation
        while (!nodes.empty())
        {
            auto* top = nodes.top();
            node_stack.push(top);
            nodes.pop();
            for (blt::size_t i = 0; i < top->type.argc(); i++)
                nodes.push(top->children[i]);
        }
        
        while (!node_stack.empty())
        {
            node_stack.top()->evaluate();
            node_stack.pop();
        }
        
        return {root->type.getValue(), root->type.getType()};
    }
    
    detail::node_t* tree_t::allocate_non_terminal(detail::node_construction_info_t info, type_id type)
    {
        const auto& non_terminals = info.types.get_non_terminals(type);
        function_id selection = non_terminals[info.engine.random_long(0, non_terminals.size() - 1)];
        func_t func(info.types.get_function_argc(selection), info.types.get_function(selection), type, selection);
        if (const auto& func_init = info.types.get_function_initializer(selection))
            func_init.value()(func);
        return info.tree.alloc.template emplace<detail::node_t>(func, info.tree.alloc);
    }
    
    detail::node_t* tree_t::allocate_terminal(detail::node_construction_info_t info, type_id type)
    {
        const auto& terminals = info.types.get_terminals(type);
        
        // if we cannot allocate a terminal, we need to allocate a non-terminal in hopes of finding a closing path
        // for example bools might not have an ending terminal, it doesn't make sense to.
        if (terminals.empty())
            return allocate_non_terminal_restricted(info, type);
        
        function_id selection = terminals[info.engine.random_long(0, terminals.size() - 1)];
        func_t func(info.types.get_function_argc(selection), info.types.get_function(selection), type, selection);
        if (const auto& func_init = info.types.get_function_initializer(selection))
            func_init.value()(func);
        return info.tree.alloc.template emplace<detail::node_t>(func, info.tree.alloc);
    }
    
    detail::node_t* tree_t::allocate_non_terminal_restricted(detail::node_construction_info_t info, type_id type)
    {
        function_id selection = 0;
        do
        {
            const auto& non_terminals = info.types.get_non_terminals(type);
            selection = info.engine.random_long(0, non_terminals.size() - 1);
            auto& sel_v = info.types.get_function_allowed_arguments(selection);
            // if it does not accept the type we are, we will accept this as a valid "temp" non-terminal
            if (std::find(sel_v.begin(), sel_v.end(), type) == sel_v.end())
                break;
        } while (true);
        
        func_t func(info.types.get_function_argc(selection), info.types.get_function(selection), type, selection);
        if (const auto& func_init = info.types.get_function_initializer(selection))
            (*func_init)(func);
        return info.tree.alloc.template emplace<detail::node_t>(func, info.tree.alloc);
    }
    
    void tree_t::grow(detail::node_construction_info_t info, blt::size_t min_depth, blt::size_t max_depth)
    {
        using namespace detail;
        std::stack<std::pair<node_t*, blt::size_t>> stack;
        stack.emplace(info.tree.root, 0);
        while (!stack.empty())
        {
            auto top = stack.top();
            auto* node = top.first;
            auto depth = top.second;
            stack.pop();
            
            const auto& allowed_types = info.types.get_function_allowed_arguments(node->type.getFunction());
            // we need to make sure there is at least one non-terminal generation, until we hit the min height
            bool has_one_non_terminal = false;
            for (blt::size_t i = 0; i < node->type.argc(); i++)
            {
                type_id type_category = allowed_types[i];
                
                if (depth < min_depth && !has_one_non_terminal)
                {
                    // make sure we have at least min height possible by using at least one non terminal
                    node->children[i] = allocate_non_terminal(info, type_category);
                    has_one_non_terminal = true;
                } else if (depth >= max_depth || info.engine.choice(info.terminal_chance))
                {
                    // if we are above the max_height select only terminals or otherwise select between use of terminals
                    node->children[i] = allocate_terminal(info, type_category);
                } else
                {
                    // and use of non-terminals method
                    node->children[i] = allocate_non_terminal(info, type_category);
                }
                // node has children that need populated
                if (node->children[i]->type.argc() != 0)
                    stack.emplace(node->children[i], depth + 1);
            }
        }
    }
    
    void tree_t::full(detail::node_construction_info_t info, blt::size_t select_depth)
    {
        using namespace detail;
        std::stack<std::pair<node_t*, blt::size_t>> stack;
        stack.emplace(info.tree.root, 0);
        while (!stack.empty())
        {
            auto top = stack.top();
            auto* node = top.first;
            auto depth = top.second;
            stack.pop();
            
            const auto& allowed_types = info.types.get_function_allowed_arguments(node->type.getFunction());
            for (blt::size_t i = 0; i < node->type.argc(); i++)
            {
                type_id type_category = allowed_types[i];
                
                if (depth >= select_depth)
                {
                    // if we are above the max_height select only terminals
                    node->children[i] = allocate_terminal(info, type_category);
                } else
                {
                    // otherwise only non-terminals can be used
                    node->children[i] = allocate_non_terminal(info, type_category);
                }
                // node has children that need populated
                if (node->children[i]->type.argc() != 0)
                    stack.emplace(node->children[i], depth + 1);
            }
        }
    }
    
    
}