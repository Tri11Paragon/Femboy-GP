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
#include <lilfbtf/test3.h>
#include <blt/std/array.h>
#include "blt/std/utility.h"
#include "blt/std/logging.h"

namespace fb
{
    class base
    {
        private:
            blt::size_t argc = 0;
            
        public:
    };
    
    void test3()
    {
        auto cum = new blt::u8[512];
        auto v = blt::array<int>::construct(cum, 512);
        auto& vr = *v;
        
        BLT_TRACE("%p", cum);
        BLT_TRACE(v);
        BLT_TRACE(*vr);
        vr[0] = 0;
        BLT_TRACE(*vr);
        vr[1] = 0;
        BLT_TRACE(*vr);
        vr[5] = 50;
        BLT_DEBUG(vr[5]);
        BLT_TRACE(*vr);
        
        blt::black_box(v);
        
        delete[] cum;
    }
}