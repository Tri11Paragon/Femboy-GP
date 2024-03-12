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
#include <vector>

namespace fb
{
    class type_engine_t
    {
        private:
            // type name -> type id
            blt::hashmap_t<std::string, type_id> name_to_id;
            // also used to assign IDs
            std::vector<std::string> id_to_name;
            // TODO: we don't need a hashmap for this.
            // Also a bad idea to store references, however these functions should be declared statically so this isn't as big of an issue.
            blt::hashmap_t<std::string, std::reference_wrapper<func_t_call_t>> functions;
            // function names -> type_id
            blt::hashmap_t<std::string, type_id> function_outputs;
            // function names -> list of type_id for parameters where index 0 = arg 1
            blt::hashmap_t<std::string, std::vector<type_id>> function_inputs;
        public:
            type_engine_t() = default;
            
            type_id register_type(type_name type_name);
            
            inline type_id get_type_id(type_name name)
            {
                return name_to_id[name];
            }
            
            type_engine_t& register_function(function_name func_name, func_t_call_t& func);
            
            type_engine_t& associate_output(function_name func_name, type_name type_name);
            
            type_engine_t& associate_input(function_name func_name, type_name)
    };
}

#endif //LILFBTF5_TYPE_H
