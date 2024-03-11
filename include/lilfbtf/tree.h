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
#include <lilfbtf/fwddecl.h>

namespace fb
{
    using func_t_call_t = std::function<void(func_t&, blt::span<detail::node_t*>)>;
    
    class func_t
    {
        private:
            blt::size_t argc_ = 0;
            std::string type;
            const func_t_call_t& func;
        protected:
            blt::unsafe::any_t value;
        public:
            explicit func_t(blt::size_t argc, const func_t_call_t& func):
                    argc_(argc), func(func)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline blt::unsafe::any_t getValue() const
            {
                return value;
            }
            
            inline func_t& setValue(blt::unsafe::any_t val)
            {
                this->value = val;
                return *this;
            }
            
            inline std::string& getType()
            {
                return type;
            }
            
            inline void call(blt::span<detail::node_t*> args)
            {
                func(*this, args);
            };
            
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
            blt::bump_allocator<blt::BLT_2MB_SIZE, false>& alloc;
        public:
        
    };
}

#endif //LILFBTF5_TREE_H
