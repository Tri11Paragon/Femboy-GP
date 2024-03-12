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
        type_id id = id_to_name.size();
        id_to_name.push_back(type_name);
        name_to_id[type_name] = id;
        return id;
    }
    
    type_engine_t& type_engine_t::register_function(function_name func_name, func_t_call_t& func)
    {
        functions.insert({func_name, func});
        return *this;
    }
    
    type_engine_t& type_engine_t::associate_output(function_name func_name, type_name type_name)
    {
        function_outputs[func_name] = get_type_id(type_name);
        return *this;
    }
}