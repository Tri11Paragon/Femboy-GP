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
#include <blt/std/vector.h>
#include <iostream>
#include "blt/std/utility.h"
#include <blt/std/ranges.h>
#include <variant>
#include <array>

namespace fb
{
    using arg_t = double;
    
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

#define STATIC_FUNCTION_LIST \
    STATIC_FUNCTION_APPLY(test_add_function_t, args[0], args[1])    \
    STATIC_FUNCTION_APPLY(test_sub_function_t, args[0], args[1])    \
    STATIC_FUNCTION_APPLY(test_mul_function_t, args[0], args[1])    \
    STATIC_FUNCTION_APPLY(test_div_function_t, args[0], args[1])    \
    STATIC_FUNCTION_APPLY(test_exp_function_t, args[0])    \
    STATIC_FUNCTION_APPLY(test_log_function_t, args[0])    \
    STATIC_FUNCTION_APPLY(test_sin_function_t, args[0])    \
    STATIC_FUNCTION_APPLY(test_cos_function_t, args[0])

#define FUNCTION_LIST \
    FUNCTION_APPLY(add_base, add_t, args[0], args[1]) \
    FUNCTION_APPLY(sub_base, sub_t, args[0], args[1]) \
    FUNCTION_APPLY(mul_base, mul_t, args[0], args[1]) \
    FUNCTION_APPLY(div_base, div_t, args[0], args[1]) \
    FUNCTION_APPLY(exp_base, exp_t, args[0]) \
    FUNCTION_APPLY(log_base, log_t, args[0]) \
    FUNCTION_APPLY(sin_base, sin_t, args[0]) \
    FUNCTION_APPLY(cos_base, cos_t, args[0])
    
    class function_virtual_t
    {
        private:
            arg_count_t args_count_;
        public:
            explicit function_virtual_t(arg_count_t args_count): args_count_(args_count)
            {}
            
            virtual arg_t operator()(blt::span<arg_t> args) = 0;
            
            [[nodiscard]] arg_count_t argCount() const noexcept
            {
                return args_count_;
            }
            
            virtual ~function_virtual_t() = default;
    };
    
    // create virtual function variants

#define FUNCTION_APPLY(type, name, ...) \
    class function_virtual_##name : public function_virtual_t{ \
        public:                          \
            function_virtual_##name(): function_virtual_t(type.argCount())\
            {}                           \
                                         \
            arg_t operator()(blt::span<arg_t> args) final \
            { \
                return type(__VA_ARGS__);\
            }\
    };
    
    FUNCTION_LIST

#undef FUNCTION_APPLY
#define FUNCTION_APPLY(type, name, ...) std::make_unique<function_virtual_##name>(),
    
    std::array<std::unique_ptr<function_virtual_t>, static_cast<int>(symbolic_regress_function_t::SIZE)> virtual_functions = {
            FUNCTION_LIST
    };

#undef FUNCTION_APPLY
    
    using function_variant = std::variant<test_add_function_t, test_sub_function_t, test_mul_function_t, test_div_function_t, test_exp_function_t, test_log_function_t, test_sin_function_t, test_cos_function_t>;

#define STATIC_FUNCTION_APPLY(type, ...) \
    constexpr auto operator()(const type& func){  \
        return func(__VA_ARGS__);   \
    }
    
    
    struct function_variant_visitor_apply
    {
        private:
            blt::span<arg_t> args;
        public:
            constexpr explicit function_variant_visitor_apply(blt::span<arg_t> args): args(args)
            {}
            
            STATIC_FUNCTION_LIST
    };

#undef STATIC_FUNCTION_APPLY
#define STATIC_FUNCTION_APPLY(type, ...) \
    constexpr auto operator()(const type& func){  \
        return func.argCount();   \
    }
    
    struct function_variant_visitor_argc
    {
        STATIC_FUNCTION_LIST
    };
    
    
    class flat_tree
    {
    
    };
    
    template<typename ENUM_TYPE>
    class arg_constraint_container
    {
        private:
            blt::vector<std::vector<ENUM_TYPE>> map;
        public:
            template<typename T, std::enable_if<std::is_convertible_v<T, blt::vector<std::vector<ENUM_TYPE>>>, bool> = true>
            constexpr explicit arg_constraint_container(T&& map): map(std::forward<T>(map))
            {}
            
            template<typename T, std::enable_if<std::is_convertible_v<T, blt::vector<ENUM_TYPE>>, bool> = true>
            constexpr explicit arg_constraint_container(blt::size_t argc, T&& map)
            {
                for (blt::size_t i = 0; i < argc; i++)
                    this->map.push_back(map);
            }
            
            constexpr arg_constraint_container(std::initializer_list<blt::vector<ENUM_TYPE>> maps)
            {
                for (const auto& v : maps)
                    this->map.push_back(v);
            }
    };
    
    template<typename ARG_TYPE, typename ENUM_TYPE>
    class operator_t
    {
        private:
            arg_count_t argc;
            std::function<ARG_TYPE(blt::span<ARG_TYPE>)> func;
            arg_constraint_container<ENUM_TYPE> allowed_inputs;
        public:
            constexpr operator_t(arg_count_t argc, std::function<ARG_TYPE(blt::span<ARG_TYPE>)> func): argc(argc), func(std::move(func))
            {}
            
            [[nodiscard]] constexpr arg_count_t argCount() const
            { return argc; }
            
            [[nodiscard]] constexpr std::function<ARG_TYPE(blt::span<ARG_TYPE>)> function() const
            { return func; }
        
        
    };
    
    template<typename NODE_CONTAINER, typename ALLOC>
    class node_tree
    {
        private:
            static ALLOC allocator;
            struct node
            {
                std::array<node*, NODE_CONTAINER::determine_max_argc()> children;
                NODE_CONTAINER caller;
            };
        public:
        
    };
    
    /*
     * Functions
     */
    
    void run_tree_type_tests(blt::size_t population_size, blt::size_t tree_min_size, blt::size_t tree_max_size)
    {
        (void) population_size;
        (void) tree_min_size;
        (void) tree_max_size;
        
        std::cout << log_base(population_size) << std::endl;
    }
}