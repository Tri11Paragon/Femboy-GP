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
            blt::hashmap_t<std::string, type_id> name_to_id;
            // also used to assign IDs
            std::vector<std::string> id_to_name;
            // TODO: we don't need a hashmap for this.
            // Also a bad idea to store references, however these functions should be declared statically so this isn't as big of an issue.
            blt::hashmap_t<type_id, std::reference_wrapper<func_t_call_t>> functions;
        public:
            type_engine_t() = default;
            
            type_id register_type(const std::string& type_name)
            {
                type_id id = id_to_name.size();
                id_to_name.push_back(type_name);
                name_to_id[type_name] = id;
                return id;
            }
            
            void register_function(const std::string& type_name, func_t_call_t& func)
            {
            
            }
    };
}

#endif //LILFBTF5_TYPE_H
