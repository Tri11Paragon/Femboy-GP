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

#ifndef LILFBTF5_TEST2_H
#define LILFBTF5_TEST2_H

#include <random>
#include <blt/std/types.h>

namespace fb
{
    void test2();
    
    static constexpr blt::u64 SEED = 691;
    
    struct random_engine
    {
        private:
            std::mt19937_64 engine{SEED};
        public:
            random_engine() = default;
            
            void reset(blt::u64 seed = SEED)
            {
                engine = std::mt19937_64{seed};
            }
            
            auto& get()
            {
                return engine;
            }
    };
    
    inline random_engine engine;
    
    enum class type_t
    {
        ADD, SUB, MUL, DIV, VALUE, END
    };
    
    inline type_t random_type()
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0, static_cast<int>(type_t::END) - 1);
        return static_cast<type_t>(dist(engine.get()));
    }
    
    inline type_t random_type_sub()
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0, static_cast<int>(type_t::END) - 2);
        return static_cast<type_t>(dist(engine.get()));
    }
    
    inline double random_value()
    {
        static std::random_device dev;
        static std::uniform_real_distribution dist(-2.0, 2.0);
        return dist(engine.get());
    }
    
    inline bool choice()
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0, 1);
        return dist(engine.get());
    }
    
}

#endif //LILFBTF5_TEST2_H
