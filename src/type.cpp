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
#include <lilfbtf/type.h>

namespace fb
{
    
    type_id type_engine_t::register_type(type_name type_name)
    {
        type_id id = type_to_name.size();
        type_to_name.push_back(type_name);
        name_to_type[type_name] = id;
        return id;
    }
    
    function_id type_engine_t::register_function(function_name func_name, type_name output, const func_t_call_t& func, arg_c_t argc,
                                                 std::optional<std::reference_wrapper<const func_t_init_t>> initializer)
    {
        function_id id = function_to_name.size();
        type_id tid = get_type_id(output);
        name_to_function[func_name] = id;
        functions.insert(id, func);
        non_terminals.at(tid).push_back(id);
        all_non_terminals.emplace_back(tid, id);
        function_argc.insert(id, argc);
        if (auto& init = initializer)
            function_initializer.insert({id, init.value()});
        return id;
    }
    
    type_engine_t& type_engine_t::associate_input(function_name func_name, const std::vector<std::string>& types)
    {
        auto id = get_function_id(func_name);
        std::vector<type_id> type_ids;
        for (const auto& v : types)
            type_ids.push_back(get_type_id(v));
        function_inputs.at(id) = std::move(type_ids);
        return *this;
    }
    
    function_id type_engine_t::register_terminal_function(function_name func_name, type_name output, const func_t_call_t& func,
                                                          std::optional<std::reference_wrapper<const func_t_init_t>> initializer)
    {
        function_id id = function_to_name.size();
        type_id tid = get_type_id(output);
        name_to_function[func_name] = id;
        functions.insert(id, func);
        terminals.at(tid).push_back(id);
        function_argc.insert(id, 0);
        if (auto& init = initializer)
            function_initializer.insert({id, init.value()});
        return id;
    }
}