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

#ifndef LILFBTF5_SYMBOL_REGRESSION_H
#define LILFBTF5_SYMBOL_REGRESSION_H

#include <lilfbtf/tests.h>

namespace fb
{
    
    enum class symbolic_regress_function_t
    {
        ADD, SUB, MUL, DIV, EXP, LOG, SIN, COS, SIZE
    };
    
    class test_add_function_t : public function_base_t<test_add_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_add_function_t(): function_base_t<test_add_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::ADD, 2)
            {}
            
            template<typename T>
            constexpr inline static T call(T a, T b)
            {
                return a + b;
            }
    };
    
    class test_sub_function_t : public function_base_t<test_sub_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_sub_function_t(): function_base_t<test_sub_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::SUB, 2)
            {}
            
            template<typename T>
            constexpr inline static T call(T a, T b)
            {
                return a - b;
            }
    };
    
    class test_mul_function_t : public function_base_t<test_mul_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_mul_function_t(): function_base_t<test_mul_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::MUL, 2)
            {}
            
            template<typename T>
            constexpr inline static T call(T a, T b)
            {
                return a * b;
            }
    };
    
    class test_div_function_t : public function_base_t<test_div_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_div_function_t(): function_base_t<test_div_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::DIV, 2)
            {}
            
            template<typename T>
            constexpr inline static T call(T a, T b)
            {
                if (b == 0)
                    return 0;
                return a / b;
            }
    };
    
    class test_exp_function_t : public function_base_t<test_exp_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_exp_function_t(): function_base_t<test_exp_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::EXP, 1)
            {}
            
            template<typename T>
            constexpr inline static T call(T a)
            {
                return std::exp(a);
            }
    };
    
    class test_log_function_t : public function_base_t<test_log_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_log_function_t(): function_base_t<test_log_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::LOG, 1)
            {}
            
            template<typename T>
            constexpr inline static T call(T a)
            {
                if (a == 0)
                    return 0;
                return std::log(a);
            }
    };
    
    class test_sin_function_t : public function_base_t<test_sin_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_sin_function_t(): function_base_t<test_sin_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::SIN, 1)
            {}
            
            template<typename T>
            constexpr inline static T call(T a)
            {
                return std::sin(a);
            }
    };
    
    class test_cos_function_t : public function_base_t<test_cos_function_t, symbolic_regress_function_t>
    {
        public:
            constexpr test_cos_function_t(): function_base_t<test_cos_function_t, symbolic_regress_function_t>(symbolic_regress_function_t::COS, 1)
            {}
            
            template<typename T>
            constexpr inline static T call(T a)
            {
                return std::cos(a);
            }
    };
    
}

#endif //LILFBTF5_SYMBOL_REGRESSION_H
