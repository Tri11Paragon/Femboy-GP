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
#include <lilfbtf/tests.h>
#include <lilfbtf/symbol_regression.h>
#include <blt/std/logging.h>
#include <iostream>
#include "blt/std/utility.h"

namespace fb
{
    
    /*
     * Classes
     */
    test_add_function_t add_base;
    test_sub_function_t sub_base;
    test_mul_function_t mul_base;
    test_div_function_t div_base;
    test_exp_function_t exp_base;
    test_log_function_t log_base;
    test_sin_function_t sin_base;
    test_cos_function_t cos_base;
    
    
    /*
     * Functions
     */
    
    void run_tree_type_tests(blt::size_t population_size, blt::size_t tree_min_size, blt::size_t tree_max_size)
    {
        (void) population_size;
        (void) tree_min_size;
        (void) tree_max_size;
        
        std::cout << exp_base(population_size) << std::endl;
    }
}