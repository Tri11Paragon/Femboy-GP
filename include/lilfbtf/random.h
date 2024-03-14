#pragma once
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

#ifndef LILFBTF5_RANDOM_H
#define LILFBTF5_RANDOM_H

#include <random>
#include <blt/std/types.h>

namespace fb
{
    class random
    {
        private:
            blt::u64 seed;
            std::mt19937_64 engine;
        public:
            explicit random(blt::u64 seed);
            
            void reset();
            bool choice();
            bool chance(double chance = 0.5);
            
            float random_float(float min = 0, float max = 1);
            double random_double(double min = 0, double max = 1);
            blt::u64 random_long(blt::u64 min = 0, blt::u64 max = 1);
            blt::i32 random_int(blt::i32 min = 0, blt::i32 max = 1);
    };
}

#endif //LILFBTF5_RANDOM_H
