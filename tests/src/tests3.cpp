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
#include <lilfbtf/test3.h>
#include <blt/std/array.h>
#include "blt/std/utility.h"
#include "blt/std/logging.h"
#include "blt/std/allocator.h"
#include "blt/profiling/profiler_v2.h"
#include <lilfbtf/test2.h>
#include <stack>

namespace fb
{
    using TYPE = double;
    
    class base_t
    {
        private:
            blt::size_t argc_ = 0;
        protected:
            TYPE value = 0;
        public:
            explicit base_t(blt::size_t argc): argc_(argc)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline TYPE getValue() const
            {
                return value;
            }
            
            inline virtual void call(blt::span<TYPE> args) = 0;
            
            virtual ~base_t() = default;
    };
    
    class add_t : public base_t
    {
        public:
            add_t(): base_t(2)
            {}
            
            inline void call(blt::span<TYPE> args) final
            {
                value = args[0] + args[1];
            }
    };
    
    class sub_t : public base_t
    {
        public:
            sub_t(): base_t(2)
            {}
            
            inline void call(blt::span<TYPE> args) final
            {
                value = args[0] - args[1];
            }
    };
    
    class mul_t : public base_t
    {
        public:
            mul_t(): base_t(2)
            {}
            
            inline void call(blt::span<TYPE> args) final
            {
                value = args[0] * args[1];
            }
    };
    
    class div_t : public base_t
    {
        public:
            div_t(): base_t(2)
            {}
            
            inline void call(blt::span<TYPE> args) final
            {
                if (args[1] == 0)
                    value = 0;
                else
                    value = args[0] / args[1];
            }
    };
    
    class value_t : public base_t
    {
        public:
            value_t(): base_t(0)
            {
                value = random_value();
            }
            
            inline void call(blt::span<TYPE>) final
            {}
    };
    
    blt::bump_allocator<blt::BLT_2MB_SIZE, true> alloc;
    
    base_t* create_node_type(type_t i)
    {
        switch (i)
        {
            case type_t::ADD:
                return alloc.emplace<add_t>();
            case type_t::SUB:
                return alloc.emplace<sub_t>();
            case type_t::MUL:
                return alloc.emplace<mul_t>();
            case type_t::DIV:
                return alloc.emplace<div_t>();
            case type_t::VALUE:
                return alloc.emplace<value_t>();
            default:
                BLT_ERROR("Hey maybe something weird is going on here");
                return nullptr;
        }
    }
    
    class tree2
    {
        private:
            struct node_t
            {
                base_t* type;
                std::array<node_t*, 2> children{nullptr};
                
                explicit node_t(type_t type): type(create_node_type(type))
                {}
                
                void evaluate()
                {
                    if (type->argc() > 0)
                    {
                        TYPE v1 = children[0]->type->getValue();
                        TYPE v2 = children[1]->type->getValue();
                        TYPE d[2]{v1, v2};
                        type->call(blt::span{d});
                    } else
                        type->call({});
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
                        for (blt::size_t i = 0; i < top->type->argc(); i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    return type->getValue();
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type->argc(); i++)
                    {
                        alloc.destroy(children[i]);
                        alloc.deallocate(children[i]);
                    }
                    alloc.destroy(type);
                    alloc.deallocate(type);
                }
            };
            
            node_t* root = nullptr;
        public:
            
            void create(blt::u64 size)
            {
                root = alloc.template emplace<node_t>(random_type());
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
                    for (blt::size_t i = 0; i < node->type->argc(); i++)
                    {
                        if (depth >= size)
                        {
                            node->children[i] = alloc.template emplace<node_t>(type_t::VALUE);
                            //BLT_INFO("Skipping due to size, value %lf", node->children[i]->value);
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc.template emplace<node_t>(random_type());
                        else
                            node->children[i] = alloc.template emplace<node_t>(random_type_sub());
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
            
            ~tree2()
            {
                BLT_START_INTERVAL("Tree Destruction", "Inheritance Tree");
                alloc.destroy(root);
                alloc.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", "Inheritance Tree");
            }
    };
    
    class tree3
    {
        private:
            struct node_t
            {
                base_t* type = nullptr;
                node_t** children = nullptr;
                type_t type_value;
                
                explicit node_t(type_t type): type(create_node_type(type)), type_value(type)
                {
                    if (this->type == nullptr)
                        throw std::bad_alloc();
                    else if (reinterpret_cast<blt::size_t>(this->type) % alignof(base_t*) != 0)
                        BLT_WARN("type pointer %p is misaligned, expecting alignment of %ld reminder %ld", this->type, alignof(base_t*),
                                 reinterpret_cast<blt::size_t>(this->type) % alignof(base_t*));
                    children = alloc.emplace_many<node_t*>(this->type->argc());
                    for (blt::size_t i = 0; i < this->type->argc(); i++)
                        children[i] = nullptr;
                    if (reinterpret_cast<blt::size_t>(this->children) % alignof(node_t**) != 0)
                        BLT_WARN("children pointer is misaligned, expecting alignment of %ld remainder %ld", this->children, alignof(node_t**),
                                 reinterpret_cast<blt::size_t>(this->children) % alignof(node_t**));
                }
                
                void evaluate() const
                {
                    if (type->argc() > 0)
                    {
                        TYPE v1 = children[0]->type->getValue();
                        TYPE v2 = children[1]->type->getValue();
                        TYPE d[2]{v1, v2};
                        type->call(blt::span{d});
                    } else
                        type->call({});
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
                        for (blt::size_t i = 0; i < top->type->argc(); i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    return type->getValue();
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type->argc(); i++)
                    {
                        alloc.destroy(children[i]);
                        alloc.deallocate(children[i]);
                    }
                    alloc.deallocate(children);
                    alloc.destroy(type);
                    alloc.deallocate(type);
                }
            };
            
            node_t* root = nullptr;
        public:
            tree3()
            {
//                BLT_INFO(alignof(node_t*));
//                BLT_TRACE(alignof(node_t*[2]));
//                BLT_DEBUG(alignof(node_t*[3]));
//                BLT_INFO(sizeof(node_t*));
//                std::exit(0);
            }
            
            void create(blt::u64 size)
            {
                root = alloc.template emplace<node_t>(random_type());
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
                    for (blt::size_t i = 0; i < node->type->argc(); i++)
                    {
                        if (depth >= size)
                        {
                            node->children[i] = alloc.template emplace<node_t>(type_t::VALUE);
                            //BLT_INFO("Skipping due to size, value %lf", node->children[i]->value);
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc.template emplace<node_t>(random_type());
                        else
                            node->children[i] = alloc.template emplace<node_t>(random_type_sub());
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
            
            ~tree3()
            {
                BLT_START_INTERVAL("Tree Destruction", "Inheritance Tree v2");
                alloc.destroy(root);
                alloc.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", "Inheritance Tree v2");
            }
    };
    
    void run()
    {
        constexpr auto size = 512;
        constexpr auto tree_size = 17;
        engine.reset();
        tree2 love[size];
        BLT_START_INTERVAL("Tree Construction", "Inheritance Tree");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", "Inheritance Tree");
        BLT_START_INTERVAL("Tree Evaluation", "Inheritance Tree");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", "Inheritance Tree");
    }
    
    void run2()
    {
        constexpr auto size = 512;
        constexpr auto tree_size = 17;
        engine.reset();
        tree3 love[size];
        BLT_START_INTERVAL("Tree Construction", "Inheritance Tree v2");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", "Inheritance Tree v2");
        BLT_START_INTERVAL("Tree Evaluation", "Inheritance Tree v2");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", "Inheritance Tree v2");
    }
    
    void test3()
    {
        auto cum = new blt::u8[512];
        auto v = blt::array<int>::construct(cum, 512);
        auto& vr = *v;
        
        BLT_TRACE("%p", cum);
        BLT_TRACE(v);
        BLT_TRACE(*vr);
        vr[0] = 0;
        BLT_TRACE(*vr);
        vr[1] = 0;
        BLT_TRACE(*vr);
        vr[5] = 50;
        BLT_DEBUG(vr[5]);
        BLT_TRACE(*vr);
        
        blt::black_box(v);
        
        delete[] cum;
        
        run();
        run2();

//        using testing = blt::size_t;
//        constexpr blt::size_t INT_SIZE = blt::BLT_2MB_SIZE * 8 / sizeof(testing);
//        auto** data = new testing*[INT_SIZE];
//
//        for (blt::size_t i = 0; i < INT_SIZE; i++)
//        {
//            data[i] = alloc.emplace<testing>();
//            *data[i] = 256;
//        }
//
//        for (blt::size_t i = 0; i < INT_SIZE; i++)
//        {
//            alloc.deallocate(data[i]);
//            auto* blk = alloc.blk(data[i]);
//            if (*data[i] != 256)
//                BLT_WARN("Data is not 256 (%d)! %ld || %ld || 0x%lx || 0x%lx pointer %p part of block %p", *data[i], i % blt::BLT_2MB_SIZE, i, i, i % blt::BLT_2MB_SIZE, data[i], blk);
//        }

//        delete[] data;
    }
}