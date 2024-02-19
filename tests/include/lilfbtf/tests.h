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

#ifndef LILFBTF5_TESTS_H
#define LILFBTF5_TESTS_H

#include "blt/std/types.h"
#include <type_traits>
#include <variant>
#include <cmath>


namespace fb
{
    
    using arg_count_t = blt::size_t;
    template<typename... allowed_arg_types>
    using arg_t = std::variant<allowed_arg_types...>;
    
    template<typename BASE, typename ENUM_TYPE, std::enable_if_t<std::is_enum_v<ENUM_TYPE>, bool> = true>
    class function_base_t
    {
        protected:
            ENUM_TYPE type_;
            arg_count_t args_count_;
        public:
            function_base_t(ENUM_TYPE type, arg_count_t args_count): type_(type), args_count_(args_count)
            {}
            
            template<typename... arg_types>
            auto operator()(arg_types&& ... args)
            {
                return BASE::call(std::forward<arg_types>(args)...);
            }
            
            ENUM_TYPE type()
            { return type_; }
            
            arg_count_t argCount()
            { return args_count_; }
    };
    
    void run_tree_type_tests(blt::size_t population_size, blt::size_t tree_min_size, blt::size_t tree_max_size);
    
    inline void execute_tests()
    {
        run_tree_type_tests(1, 5, 17);
        run_tree_type_tests(10, 5, 17);
        run_tree_type_tests(100, 5, 17);
        run_tree_type_tests(1000, 5, 17);
        run_tree_type_tests(10000, 5, 17);
        run_tree_type_tests(100000, 5, 17);
        run_tree_type_tests(1000000, 5, 17);
    }
    
}

#endif //LILFBTF5_TESTS_H
