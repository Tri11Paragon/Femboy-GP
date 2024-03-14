#pragma once
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

#ifndef LILFBTF5_TREE_H
#define LILFBTF5_TREE_H

#include <blt/std/any.h>
#include <functional>
#include "blt/std/ranges.h"
#include "blt/std/allocator.h"
#include "type.h"
#include <lilfbtf/fwddecl.h>
#include <lilfbtf/random.h>

namespace fb
{
    
    class func_t
    {
        private:
            arg_c_t argc_ = 0;
            type_id type;
            function_id function;
            const func_t_call_t& func;
        protected:
            blt::unsafe::any_t value;
        public:
            explicit func_t(arg_c_t argc, const func_t_call_t& func, type_id output_type, function_id function_type);
            
            [[nodiscard]] inline arg_c_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline blt::unsafe::any_t getValue() const
            { return value; }
            
            inline func_t& setValue(blt::unsafe::any_t val)
            {
                this->value = val;
                return *this;
            }
            
            /**
             * @return the type_id that this function container will output
             */
            [[nodiscard]] inline type_id getType() const
            { return type; }
            
            /**
             * @return the function id of this function container
             */
            [[nodiscard]] inline function_id getFunction() const
            { return function; }
            
            inline void call(blt::span<detail::node_t*> args)
            { func(*this, args); };
            
            ~func_t() = default;
    };
    
    namespace detail
    {
        class node_t
        {
                friend tree_t;
            private:
                blt::bump_allocator<blt::BLT_2MB_SIZE, false>& alloc;
                func_t type;
                node_t** children = nullptr;
            public:
                explicit node_t(const func_t& type, blt::bump_allocator<blt::BLT_2MB_SIZE, false>& alloc): alloc(alloc), type(type)
                {
                    children = alloc.emplace_many<node_t*>(this->type.argc());
                    for (blt::size_t i = 0; i < this->type.argc(); i++)
                        children[i] = nullptr;
                }
                
                inline void evaluate()
                {
                    type.call(blt::span<node_t*>{children, type.argc()});
                }
                
                inline blt::unsafe::any_t value()
                {
                    return type.getValue();
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type.argc(); i++)
                    {
                        alloc.destroy(children[i]);
                        alloc.deallocate(children[i]);
                    }
                    alloc.deallocate(children, type.argc());
                }
        };
    }
    
    class tree_t
    {
        private:
            blt::bump_allocator<blt::BLT_2MB_SIZE, false> alloc;
            type_engine_t& types;
            detail::node_t* root = nullptr;
            
            inline blt::bump_allocator<blt::BLT_2MB_SIZE, false>& get_allocator()
            { return alloc; }
        public:
            explicit tree_t(type_engine_t& types);
            
            static tree_t make_tree(type_engine_t& types, random& engine, blt::size_t min_height, blt::size_t max_height, std::optional<type_id> starting_type = {});
            
            std::pair<blt::unsafe::any_t, type_id> evaluate();
    };
}

#endif //LILFBTF5_TREE_H
