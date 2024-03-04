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
    
    void execute()
    {
    
    }
    
    enum class type_t
    {
        ADD, SUB, MUL, VALUE
    };
    
    type_t random_type()
    {
        static std::random_device dev;
        static std::mt19937_64 engine{dev()};
        static std::uniform_int_distribution dist(0, 3);
        return static_cast<type_t>(dist(engine));
    }
    
    type_t random_type_sub()
    {
        static std::random_device dev;
        static std::mt19937_64 engine{dev()};
        static std::uniform_int_distribution dist(0, 2);
        return static_cast<type_t>(dist(engine));
    }
    
    double random_value()
    {
        static std::random_device dev;
        static std::mt19937_64 engine{dev()};
        static std::uniform_real_distribution dist(-10.0, 10.0);
        return dist(engine);
    }
    
    bool choice()
    {
        static std::random_device dev;
        static std::mt19937_64 engine{dev()};
        static std::uniform_int_distribution dist(0, 1);
        return dist(engine);
    }
    
    std::array<blt::i32, 4> arg_c = {2, 2, 2, 0};
    
    struct construct_info
    {
        type_t type;
        bool choice;
    };
    
    std::vector<construct_info> create_info(blt::size_t size)
    {
        std::vector<construct_info> info;
        
        construct_info root = {random_type(), false};
        std::stack<std::pair<construct_info, blt::size_t>> stack;
        stack.emplace(root, 0);
        while (!stack.empty())
        {
            auto top = stack.top();
            auto node = top.first;
            auto depth = top.second;
            info.push_back(node);
            stack.pop();
            for (blt::i32 i = 0; i < arg_c[static_cast<int>(info.back().type)]; i++)
            {
                if (depth >= size)
                    break;
                if (choice())
                    stack.emplace(construct_info{random_type(), true}, depth + 1);
                else
                    stack.emplace(construct_info{random_type_sub(), false}, depth + 1);
            }
        }
        
        return info;
    }
    
    class tree1
    {
        public:
            blt::bump_allocator<true> alloc{sizeof(node_t) * 8192};
        private:
            struct node_t
            {
                blt::bump_allocator<true>& alloc;
                std::array<node_t*, 2> children{};
                double value = 0;
                blt::i32 argc;
                type_t type;
                
                explicit node_t(type_t type, blt::bump_allocator<true>& alloc): alloc(alloc), argc(arg_c[static_cast<int>(type)]), type(type)
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
            
            void create(const std::vector<construct_info>& info)
            {
//                root = alloc.emplace<node_t>(random_type(), alloc);
//                std::stack<std::pair<node_t*, blt::size_t>> stack;
//                stack.emplace(root, 0);
//                while (!stack.empty())
//                {
//                    auto top = stack.top();
//                    auto* node = top.first;
//                    auto depth = top.second;
//                    stack.pop();
//                    for (blt::i32 i = 0; i < node->argc; i++)
//                    {
//                        auto& assignment = node->children[i];
//                        if (choice())
//                            assignment = alloc.emplace<node_t>(random_type(), alloc);
//                        else
//                            assignment = alloc.emplace<node_t>(random_type_sub(), alloc);
//                        if (depth < size)
//                            stack.emplace(assignment, depth + 1);
//                    }
//                }
                root = alloc.emplace<node_t>(info[0].type, alloc);
                blt::size_t index = 1;
                std::stack<std::pair<node_t*, blt::size_t>> stack;
                stack.emplace(root, 0);
                while (!stack.empty())
                {
                    auto top = stack.top();
                    auto* node = top.first;
                    auto depth = top.second;
                    stack.pop();
                    for (blt::i32 i = 0; i < node->argc; i++)
                    {
                        auto& assignment = node->children[i];
                        assignment = alloc.emplace<node_t>(info[index++].type, alloc);
                        stack.emplace(assignment, depth + 1);
                    }
                }
            }
            
            double evaluate()
            {
                return root->evaluate_tree();
            }
            
            ~tree1()
            {
                alloc.destroy(root);
                alloc.deallocate(root);
            }
    };
    
    struct tree2
    {
        private:
            struct node_t
            {
                double value;
                blt::i32 argc;
                type_t type;
            };
            node_t* nodes = nullptr;
            blt::size_t size = 0;
        public:
            tree2() = default;
            
            void create(const std::vector<construct_info>& info)
            {
                size = static_cast<blt::size_t>(std::pow(2, std::log2(info.size()) + 1));
                nodes = new node_t[size];
                for (blt::size_t i = 0; i < info; i++)
                {
                
                }
            }
            
            ~tree2()
            {
                delete[] nodes;
            }
    };
    
    void funny()
    {
        auto info = create_info(25);
        tree1 love;
        love.create(info);
        BLT_TRACE(love.evaluate());
    }
}