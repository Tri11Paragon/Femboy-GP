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
#include <lilfbtf/tree.h>

namespace fb
{
    
    func_t::func_t(blt::size_t argc, const func_t_call_t& func, type_id output_type, function_id function_type):
            argc_(argc), type(output_type), function(function_type), func(func)
    {}
    
    tree_t::tree_t(blt::bump_allocator<blt::BLT_2MB_SIZE, false>& alloc, type_engine_t& types): alloc(alloc), types(types)
    {}
    
    
}