/*
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
#include <blt/std/any.h>
#include <blt/std/logging.h>
#include <lilfbtf/test5.h>

namespace fb
{
    
    struct destoryable
    {
        std::string name;
        destoryable(std::string_view name): name(name)
        {
            BLT_INFO("Called constructor %s", this->name.c_str());
        }
        
        ~destoryable()
        {
            BLT_INFO("Called destructor %s", name.c_str());
        }
    };
    
    void test5()
    {
        blt::unsafe::any_t_union any_construct(destoryable{"construct"});
        
        blt::unsafe::any_t_union any_assignment = 5;
        
        any_assignment = destoryable{"assignment"};
        
        any_construct = destoryable{"copy"};
        
        // copy
        any_assignment = any_construct;
        
        any_assignment = destoryable{"move"};
        
        any_construct = std::move(any_assignment);
        
        any_assignment = 5;
    }
    
}