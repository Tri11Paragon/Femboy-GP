/*
 *  <Short Description>
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
#include <lilfbtf/test2.h>
#include <functional>
#include <blt/std/types.h>
#include "blt/std/ranges.h"
#include "blt/std/allocator.h"
#include "blt/profiling/profiler_v2.h"
#include <random>
#include <vector>
#include <stack>

namespace fb
{
    template<typename arg_type, blt::size_t MAX_ARGS>
    class tree
    {
            using index = blt::u32;
        private:
            struct node
            {
                index parent;
                std::array<index, MAX_ARGS> children;
                std::function<arg_type(blt::span<arg_type>)> func;
            };
            std::vector<node> nodes;
        
        public:
    };
    
    std::array<blt::i32, static_cast<int>(type_t::END)> arg_c = {2, 2, 2, 2, 0};
    
    template<typename ALLOC>
    class tree1
    {
        public:
            ALLOC alloc{sizeof(node_t) * 8192};
        private:
            struct node_t
            {
                ALLOC& alloc;
                std::array<node_t*, 2> children{nullptr};
                double value = 0;
                blt::i32 argc;
                type_t type;
                
                explicit node_t(type_t type, ALLOC& alloc):
                        alloc(alloc), argc(arg_c[static_cast<int>(type)]), type(type)
                {
                    if (type == type_t::VALUE)
                        value = random_value();
                }
                
                void evaluate()
                {
                    switch (type)
                    {
                        case type_t::ADD:
                            value = children[0]->value + children[1]->value;
                            return;
                        case type_t::SUB:
                            value = children[0]->value - children[1]->value;
                            return;
                        case type_t::MUL:
                            value = children[0]->value * children[1]->value;
                            return;
                        case type_t::VALUE:
                            return;
                        case type_t::DIV:
                            if (children[1]->value == 0)
                                value = 0;
                            else
                                value = children[0]->value / children[1]->value;
                            break;
                        case type_t::END:
                            break;
                    }
                }
                
                double evaluate_tree()
                {
                    std::stack<node_t*> nodes;
                    std::stack<node_t*> node_stack;
                    
                    nodes.push(this);
                    
                    while (!nodes.empty())
                    {
                        auto* top = nodes.top();
                        node_stack.push(top);
                        nodes.pop();
                        for (blt::i32 i = 0; i < top->argc; i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    return value;
                }
                
                ~node_t()
                {
                    for (int i = 0; i < argc; i++)
                    {
                        alloc.destroy(children[i]);
                        alloc.deallocate(children[i]);
                    }
                }
            };
            
            node_t* root = nullptr;
        public:
            
            void create(blt::u64 size)
            {
                root = alloc.template emplace<node_t>(random_type(), alloc);
                std::stack<std::pair<node_t*, blt::size_t>> stack;
                stack.emplace(root, 0);
                while (!stack.empty())
                {
                    auto top = stack.top();
                    auto* node = top.first;
                    auto depth = top.second;
                    //BLT_WARN("gen type %ld with argc: %ld", node->type, node->argc);
                    stack.pop();
                    //BLT_TRACE0_STREAM << "Size: " << stack.size() << "\n";
                    for (blt::i32 i = 0; i < node->argc; i++)
                    {
                        if (depth >= size)
                        {
                            node->children[i] = alloc.template emplace<node_t>(type_t::VALUE, alloc);
                            //BLT_INFO("Skipping due to size, value %lf", node->children[i]->value);
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc.template emplace<node_t>(random_type(), alloc);
                        else
                            node->children[i] = alloc.template emplace<node_t>(random_type_sub(), alloc);
                        //BLT_INFO("child %p to %p has type generated %ld with argc %d, value %lf", node->children[i], node,
                        //         static_cast<int>(node->children[i]->type), node->children[i]->argc, node->children[i]->value);
                        if (depth < size)
                            stack.emplace(node->children[i], depth + 1);
                    }
                    //BLT_TRACE0_STREAM << "Size: " << stack.size() << "\n";
                }
//                BLT_INFO("We have %ld adds, %ld subs, %ld mul, %ld div, %ld val, == %ld", t1_add, t1_sub, t1_mul, t1_div, t1_val,
//                         t1_add + t1_sub + t1_mul + t1_val + t1_div);
            }
            
            double evaluate()
            {
                return root->evaluate_tree();
            }
            
            ~tree1()
            {
                BLT_START_INTERVAL("Tree Destruction", blt::type_string<ALLOC>() + ": Single Class Tree");
                alloc.destroy(root);
                alloc.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", blt::type_string<ALLOC>() + ": Single Class Tree");
            }
    };
    
    template<typename ALLOC>
    void bump()
    {
        constexpr auto size = 512;
        constexpr auto tree_size = 17;
        engine.reset();
        tree1<ALLOC> love[size];
        BLT_START_INTERVAL("Tree Construction", blt::type_string<ALLOC>() + ": Single Class Tree");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", blt::type_string<ALLOC>() + ": Single Class Tree");
        BLT_START_INTERVAL("Tree Evaluation", blt::type_string<ALLOC>() + ": Single Class Tree");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", blt::type_string<ALLOC>() + ": Single Class Tree");
    }
    
    void test2()
    {
        bump<blt::bump_allocator_old<true>>();
        bump<blt::bump_allocator<4096 * 512, true, 4096 * 512>>();
        bump<blt::bump_allocator<4096 * 512, false, 4096 * 512>>();
        
        BLT_PRINT_PROFILE("Tree Construction");
        BLT_PRINT_PROFILE("Tree Evaluation");
        BLT_PRINT_PROFILE("Tree Destruction");
    }
}