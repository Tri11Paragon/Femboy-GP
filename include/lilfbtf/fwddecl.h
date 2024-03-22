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

#ifndef LILFBTF5_FWDDECL_H
#define LILFBTF5_FWDDECL_H

#include <blt/std/types.h>
#include <blt/std/any.h>
#include <functional>
#include "blt/std/ranges.h"
#include <string>

namespace fb
{
    class func_t;
    
    class tree_t;
    
    class type_engine_t;
    
    class gp_system_t;
    
    class gp_population_t;
    
    namespace detail
    {
        class node_t;
        
        struct fitness_results
        {
            double fitness;
            blt::size_t hits;
        };
        
        struct func_t_arguments
        {
            // reference to ourselves
            func_t& self;
            // list of arguments to use
            blt::span<detail::node_t*> arguments;
            // any extra information that the tree evaluator wants to provide to us, in the case of an image GP this is going to be the X and Y coords
            blt::unsafe::buffer_any_t extra_args;
        };
    }
    
    // no way we are going to have more than 4billion types or functions.
    using type_id = blt::u32;
    using function_id = blt::u32;
    using arg_c_t = blt::size_t;
    using func_t_call_t = std::function<void(const detail::func_t_arguments&)>;
    using func_t_init_t = std::function<void(func_t&)>;
    using fitness_eval_func_t = std::function<detail::fitness_results(detail::node_t*)>;
    using function_name = const std::string&;
    using type_name = const std::string&;
}

#endif //LILFBTF5_FWDDECL_H
