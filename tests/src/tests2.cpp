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
    
    enum class type_t
    {
        ADD, SUB, MUL, DIV, VALUE, END
    };
    
    static constexpr blt::u64 SEED = 691;
    
    struct random_engine
    {
        private:
            std::mt19937_64 engine{SEED};
        public:
            random_engine() = default;
            
            void reset(blt::u64 seed = SEED)
            {
                engine = std::mt19937_64{seed};
            }
            
            auto& get()
            {
                return engine;
            }
    } engine;
    
    type_t random_type()
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0, static_cast<int>(type_t::END) - 1);
        return static_cast<type_t>(dist(engine.get()));
    }
    
    type_t random_type_sub()
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0, static_cast<int>(type_t::END) - 2);
        return static_cast<type_t>(dist(engine.get()));
    }
    
    double random_value()
    {
        static std::random_device dev;
        static std::uniform_real_distribution dist(-2.0, 2.0);
        return dist(engine.get());
    }
    
    bool choice()
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0, 1);
        return dist(engine.get());
    }
    
    std::array<blt::i32, static_cast<int>(type_t::END)> arg_c = {2, 2, 2, 2, 0};
    blt::size_t t1_add = 0;
    blt::size_t t1_sub = 0;
    blt::size_t t1_mul = 0;
    blt::size_t t1_div = 0;
    blt::size_t t1_val = 0;
    blt::size_t t2_add = 0;
    blt::size_t t2_sub = 0;
    blt::size_t t2_mul = 0;
    blt::size_t t2_val = 0;
    
    class tree1
    {
        public:
            blt::bump_allocator<true> alloc{sizeof(node_t) * 8192};
        private:
            struct node_t
            {
                blt::bump_allocator<true>& alloc;
                std::array<node_t*, 2> children{nullptr};
                double value = 0;
                blt::i32 argc;
                type_t type;
                
                explicit node_t(type_t type, blt::bump_allocator<true>& alloc):
                        alloc(alloc), argc(arg_c[static_cast<int>(type)]), type(type)
                {
                    if (type == type_t::VALUE)
                        value = random_value();
                    switch (type)
                    {
                        case type_t::ADD:
                            t1_add++;
                            break;
                        case type_t::SUB:
                            t1_sub++;
                            break;
                        case type_t::MUL:
                            t1_mul++;
                            break;
                        case type_t::DIV:
                            t1_div++;
                            break;
                        case type_t::VALUE:
                            t1_val++;
                            break;
                        case type_t::END:
                            break;
                    }
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
                    blt::size_t evals = 0;
                    
                    nodes.push(this);
                    
                    while (!nodes.empty())
                    {
                        auto* top = nodes.top();
                        node_stack.push(top);
                        nodes.pop();
                        //BLT_INFO("%ld type %d", top->argc, static_cast<int>(top->type));
                        for (blt::i32 i = 0; i < top->argc; i++)
                        {
                            //BLT_TRACE("Child %p", top->children[i]);
                            nodes.push(top->children[i]);
                        }
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        //BLT_DEBUG(node_stack.top()->value);
                        evals++;
                        node_stack.pop();
                    }
//                    BLT_INFO("Evaluated %ld times", evals);
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
                root = alloc.emplace<node_t>(random_type(), alloc);
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
                            node->children[i] = alloc.emplace<node_t>(type_t::VALUE, alloc);
                            //BLT_INFO("Skipping due to size, value %lf", node->children[i]->value);
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc.emplace<node_t>(random_type(), alloc);
                        else
                            node->children[i] = alloc.emplace<node_t>(random_type_sub(), alloc);
                        //BLT_INFO("child %p to %p has type generated %ld with argc %d, value %lf", node->children[i], node,
                        //         static_cast<int>(node->children[i]->type), node->children[i]->argc, node->children[i]->value);
                        if (depth < size)
                            stack.emplace(node->children[i], depth + 1);
                    }
                    //BLT_TRACE0_STREAM << "Size: " << stack.size() << "\n";
                }
//                BLT_INFO("We have %ld adds, %ld subs, %ld mul, %ld div, %ld val, == %ld", t1_add, t1_sub, t1_mul, t1_div, t1_val,
//                         t1_add + t1_sub + t1_mul + t1_val + t1_div);
                t1_add = 0;
                t1_sub = 0;
                t1_mul = 0;
                t1_div = 0;
                t1_val = 0;
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
    
    void funny()
    {
        constexpr auto size = 512;
        constexpr auto tree_size = 17;
        engine.reset();
        tree1 love[size];
        for (auto& i : love)
            i.create(tree_size);
        std::string val;
        val.reserve(size * 128 * 2);
        BLT_START_INTERVAL("Tree Evaluation", "Single Class Bump Allocated Tree");
        for (auto& i : love)
        {
            val += static_cast<char>(static_cast<long>(i.evaluate()));
            val += '\n';
        }
        BLT_END_INTERVAL("Tree Evaluation", "Single Class Bump Allocated Tree");
        BLT_TRACE(val);
        BLT_PRINT_PROFILE("Tree Evaluation");
    }
    
    void execute()
    {
    
    }
}