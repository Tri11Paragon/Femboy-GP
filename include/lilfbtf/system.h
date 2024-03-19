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

#ifndef LILFBTF5_SYSTEM_H
#define LILFBTF5_SYSTEM_H

#include <lilfbtf/fwddecl.h>
#include <lilfbtf/tree.h>
#include <vector>

namespace fb
{
    
    class gp_population_t
    {
    
    };
    
    class gp_system_t
    {
        private:
        
        public:
            gp_system_t(type_engine_t& types): types(types)
            {}
        
        private:
            type_engine_t& types;
    };
    
}

#endif //LILFBTF5_SYSTEM_H
