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
    
    tree_t tree_t::make_tree(type_engine_t& types, random& engine,
                             blt::size_t min_height, blt::size_t max_height, std::optional<type_id> starting_type)
    {
        using detail::node_t;
        tree_t tree(types);
        
        {
            if (starting_type)
            {
                auto& non_terminals = types.get_non_terminals(starting_type.value());
                auto selection = non_terminals[engine.random_long(0, non_terminals.size() - 1)];
                func_t func(types.get_function_argc(selection), types.get_function(selection), starting_type.value(), selection);
                if (const auto& func_init = types.get_function_initializer(selection))
                    func_init.value()(func);
                tree.root = tree.alloc.template emplace<node_t>(func, tree.alloc);
            } else
            {
                auto& non_terminals = types.get_all_non_terminals();
                auto selection = non_terminals[engine.random_long(0, non_terminals.size() - 1)];
                func_t func(types.get_function_argc(selection.second), types.get_function(selection.second), selection.first, selection.second);
                if (const auto& func_init = types.get_function_initializer(selection.second))
                    func_init.value()(func);
                tree.root = tree.alloc.template emplace<node_t>(func, tree.alloc);
            }
        }
        std::stack<std::pair<node_t*, blt::size_t>> stack;
        stack.emplace(tree.root, 0);
        while (!stack.empty())
        {
            auto top = stack.top();
            auto* node = top.first;
            auto depth = top.second;
            stack.pop();
            
            const auto& allowed_types = types.get_function_allowed_arguments(node->type.getFunction());
            // we need to make sure there is at least one non-terminal generation, until we hit the min height
            bool has_one_non_terminal = false;
            for (blt::size_t i = 0; i < node->type.argc(); i++)
            {
                type_id type_category = allowed_types[i];
                const auto& terminals = types.get_terminals(type_category);
                const auto& non_terminals = types.get_non_terminals(type_category);
                
                if (depth < max_height)
                    stack.emplace(node->children[i], depth + 1);
                
                if (depth < min_height && !has_one_non_terminal)
                {
                    function_id selection = non_terminals[engine.random_long(0, non_terminals.size() - 1)];
                    func_t func(types.get_function_argc(selection), types.get_function(selection), type_category, selection);
                    if (const auto& func_init = types.get_function_initializer(selection))
                        func_init.value()(func);
                    node->children[i] = tree.alloc.template emplace<node_t>(func, tree.alloc);
                    has_one_non_terminal = true;
                    continue;
                }
                
                if (depth >= max_height)
                {
                    function_id selection = terminals[engine.random_long(0, terminals.size() - 1)];
                    func_t func(types.get_function_argc(selection), types.get_function(selection), type_category, selection);
                    if (const auto& func_init = types.get_function_initializer(selection))
                        func_init.value()(func);
                    node->children[i] = tree.alloc.template emplace<node_t>(func, tree.alloc);
                    continue;
                }
                
                if (engine.choice())
                {
                    // use full() method
                    function_id selection = non_terminals[engine.random_long(0, non_terminals.size() - 1)];
                    func_t func(types.get_function_argc(selection), types.get_function(selection), type_category, selection);
                    if (const auto& func_init = types.get_function_initializer(selection))
                        func_init.value()(func);
                    node->children[i] = tree.alloc.template emplace<node_t>(func, tree.alloc);
                } else
                {
                    // use grow() method, meaning select choice again
                    if (engine.choice())
                    {
                        // use non-terminals
                        function_id selection = non_terminals[engine.random_long(0, non_terminals.size() - 1)];
                        func_t func(types.get_function_argc(selection), types.get_function(selection), type_category, selection);
                        if (const auto& func_init = types.get_function_initializer(selection))
                            func_init.value()(func);
                        node->children[i] = tree.alloc.template emplace<node_t>(func, tree.alloc);
                    } else
                    {
                        // use terminals
                        function_id selection = terminals[engine.random_long(0, terminals.size() - 1)];
                        func_t func(types.get_function_argc(selection), types.get_function(selection), type_category, selection);
                        if (const auto& func_init = types.get_function_initializer(selection))
                            func_init.value()(func);
                        node->children[i] = tree.alloc.template emplace<node_t>(func, tree.alloc);
                    }
                }
            }
        }
        
        return tree;
    }
    
    std::pair<blt::unsafe::any_t, type_id> tree_t::evaluate()
    {
        using detail::node_t;
        std::stack<node_t*> nodes;
        std::stack<node_t*> node_stack;
        
        nodes.push(root);
        
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
    
    
}