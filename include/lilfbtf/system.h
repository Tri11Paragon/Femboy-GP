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
#include <blt/std/thread.h>
#include <vector>

namespace fb
{
    
    enum class population_init_t
    {
        GROW, FULL, BRETT_GROW, RAMPED_HALF_HALF, RAMPED_TRI_HALF
    };
    
    class gp_population_t
    {
        private:
            blt::thread_pool<true>& pool;
            std::vector<tree_t> population;
            fb::random& engine;
            type_engine_t& types;
            
            void crossover();
            
            void mutate();
        
        public:
            explicit gp_population_t(blt::thread_pool<true>& pool, type_engine_t& types, fb::random& engine): pool(pool), engine(engine), types(types)
            {}
            
            void init_pop(population_init_t init_type, blt::size_t pop_size, blt::size_t min_depth, blt::size_t max_depth,
                          std::optional<type_id> starting_type = {}, double terminal_chance = 0.5);
            
            void execute(const individual_eval_func_t& individualEvalFunc, const fitness_eval_func_t& fitnessEvalFunc);
            
            void breed_new_pop();
    };
    
    class gp_system_t
    {
        private:
        
        public:
            explicit gp_system_t(type_engine_t& types): types(types)
            {}
        
        private:
            type_engine_t& types;
    };
    
}

#endif //LILFBTF5_SYSTEM_H
