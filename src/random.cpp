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
#include <lilfbtf/random.h>

namespace fb
{
    
    random::random(blt::u64 seed): seed(seed), engine(seed)
    {
    
    }
    
    void random::reset()
    {
        engine.seed(seed);
    }
    
    bool random::choice()
    {
        static std::uniform_int_distribution<int> dist(0, 1);
        return dist(engine);
    }
    
    float random::random_float(float min, float max)
    {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(engine);
    }
    
    double random::random_double(double min, double max)
    {
        std::uniform_real_distribution<double> dist(min, max);\
        return dist(engine);
    }
    
    blt::u64 random::random_long(blt::u64 min, blt::u64 max)
    {
        std::uniform_int_distribution<blt::u64> dist(min, max);
        return dist(engine);
    }
    
    blt::i32 random::random_int(blt::i32 min, blt::i32 max)
    {
        std::uniform_int_distribution<blt::i32> dist(min, max);
        return dist(engine);
    }
    
    bool random::chance(double chance)
    {
        return random_double() <= chance;
    }
}