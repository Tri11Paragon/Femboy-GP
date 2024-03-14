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

#ifndef LILFBTF5_TYPE_H
#define LILFBTF5_TYPE_H

#include <lilfbtf/fwddecl.h>
#include <blt/std/hashmap.h>
#include <blt/std/memory_util.h>
#include <vector>
#include <cstdlib>
#include <optional>

namespace fb
{
    template<typename K, typename T, bool init = false>
    class associative_array
    {
        private:
            K size_;
            T* data_;
            
            void expand()
            {
                K new_size = static_cast<K>(size_ == 0 ? 16 : blt::mem::next_byte_allocation(size_));
                T* new_data = static_cast<T*>(std::malloc(new_size * sizeof(T)));
                if constexpr (init)
                {
                    for (blt::size_t i = size_; i < new_size; i++)
                        new(&new_size[i]) T();
                }
                for (blt::size_t i = 0; i < size_; i++)
                    new(&new_data[i]) T(std::move(data_[i]));
                std::free(data_);
                data_ = new_data;
                size_ = new_size;
            }
        
        public:
            associative_array(): size_(0), data_(nullptr)
            {}
            
            T& operator[](K index)
            {
                return data_[index];
            }
            
            const T& operator[](K index) const
            {
                return data_[index];
            }
            
            void insert(K index, const T& t)
            {
                while (index >= size_)
                    expand();
                data_[index] = t;
            }
            
            void insert(K index, T&& t)
            {
                while (index >= size_)
                    expand();
                data_[index] = std::move(t);
            }
            
            [[nodiscard]] inline T* data()
            {
                return data_;
            }
            
            [[nodiscard]] inline K size() const
            {
                return size_;
            }
            
            ~associative_array()
            {
                std::free(data_);
            }
    };
    
    class type_engine_t
    {
        private:
            // type name -> type id
            blt::hashmap_t<std::string, type_id> name_to_type;
            // also used to assign IDs
            std::vector<std::string> type_to_name;
            
            blt::hashmap_t<std::string, function_id> name_to_function;
            std::vector<std::string> function_to_name;
            
            // Also a bad idea to store references, however these functions should be declared statically so this isn't as big of an issue.
            associative_array<function_id, std::reference_wrapper<const func_t_call_t>> functions;
            // function id -> list of type_id for parameters where index 0 = arg 1
            associative_array<function_id, std::vector<type_id>> function_inputs;
            associative_array<function_id, arg_c_t> function_argc;
            
            associative_array<function_id, std::reference_wrapper<const func_t_init_t>> function_initializer;
            associative_array<type_id, std::vector<function_id>, true> terminals;
            associative_array<type_id, std::vector<function_id>, true> non_terminals;
            std::vector<std::pair<type_id, function_id>> all_non_terminals;
        public:
            type_engine_t() = default;
            
            type_id register_type(type_name type_name);
            
            function_id register_function(function_name func_name, type_name output, const func_t_call_t& func, arg_c_t argc,
                                          std::optional<std::reference_wrapper<const func_t_init_t>> initializer = {});
            
            function_id register_terminal_function(function_name func_name, type_name output, const func_t_call_t& func,
                                                   std::optional<std::reference_wrapper<const func_t_init_t>> initializer = {});
            
            [[nodiscard]] inline type_id get_type_id(type_name name) const
            { return name_to_type.at(name); }
            
            [[nodiscard]] inline type_id get_function_id(function_name name) const
            { return name_to_function.at(name); }
            
            [[nodiscard]] inline arg_c_t get_function_argc(function_id id) const
            { return function_argc[id]; }
            
            [[nodiscard]] inline arg_c_t get_function_argc(function_name name) const
            { return get_function_argc(get_function_id(name)); }
            
            type_engine_t& associate_input(function_name func_name, const std::vector<std::string>& types);
            
            [[nodiscard]] inline const func_t_call_t& get_function(function_id id) const
            { return functions[id]; }
            
            [[nodiscard]] inline const func_t_call_t& get_function(function_name name) const
            { return get_function(get_function_id(name)); }
            
            [[nodiscard]] inline const func_t_init_t& get_function_initializer(function_id id) const
            { return function_initializer[id]; }
            
            [[nodiscard]] inline const func_t_init_t& get_function_initializer(function_name name) const
            { return get_function_initializer(get_function_id(name)); }
            
            inline std::vector<function_id>& get_terminals(type_id type)
            { return terminals[type]; }
            
            inline std::vector<function_id>& get_non_terminals(type_id type)
            { return non_terminals[type]; }
    };
}

#endif //LILFBTF5_TYPE_H
