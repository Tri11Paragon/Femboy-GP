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
#include <lilfbtf/system.h>

namespace fb
{
    void fb::gp_population_t::crossover()
    {
    
    }
    
    void fb::gp_population_t::mutate()
    {
    
    }
    
    void fb::gp_population_t::init_pop(const population_init_t init_type, blt::size_t pop_size, blt::size_t min_depth, blt::size_t max_depth,
                                       std::optional<type_id> starting_type, double terminal_chance)
    {
        for (blt::size_t i = 0; i < pop_size; i++)
        {
            switch (init_type)
            {
                case population_init_t::GROW:
                    population.push_back(
                            fb::tree_t::make_tree({fb::tree_init_t::GROW, engine, types, terminal_chance}, min_depth, max_depth, starting_type));
                    break;
                case population_init_t::FULL:
                    population.push_back(
                            fb::tree_t::make_tree({fb::tree_init_t::FULL, engine, types, terminal_chance}, min_depth, max_depth, starting_type));
                    break;
                case population_init_t::BRETT_GROW:
                    population.push_back(
                            fb::tree_t::make_tree({fb::tree_init_t::BRETT_GROW, engine, types, terminal_chance}, min_depth, max_depth,
                                                  starting_type));
                    break;
                case population_init_t::RAMPED_HALF_HALF:
                    if (engine.choice())
                    {
                        population.push_back(
                                fb::tree_t::make_tree({fb::tree_init_t::GROW, engine, types, terminal_chance}, min_depth, max_depth, starting_type));
                    } else
                    {
                        // will select between min and max
                        population.push_back(
                                fb::tree_t::make_tree({fb::tree_init_t::FULL, engine, types, terminal_chance}, min_depth, max_depth, starting_type));
                    }
                    break;
                case population_init_t::RAMPED_TRI_HALF:
                    if (engine.choice(0.3))
                    {
                        population.push_back(
                                fb::tree_t::make_tree({fb::tree_init_t::GROW, engine, types, terminal_chance}, min_depth, max_depth, starting_type));
                    } else if (engine.choice(0.3))
                    {
                        population.push_back(
                                fb::tree_t::make_tree({fb::tree_init_t::FULL, engine, types, terminal_chance}, min_depth, max_depth, starting_type));
                    } else
                    {
                        population.push_back(
                                fb::tree_t::make_tree({fb::tree_init_t::BRETT_GROW, engine, types, terminal_chance}, min_depth, max_depth,
                                                      starting_type));
                    }
                    break;
            }
        }
    }
    
    void fb::gp_population_t::execute(const individual_eval_func_t& individualEvalFunc, const fitness_eval_func_t& fitnessEvalFunc)
    {
        for (auto& individual : population)
        {
            individualEvalFunc(individual);
            individual.cache.fitness = fitnessEvalFunc(individual.root);
        }
    }
    
    void fb::gp_population_t::breed_new_pop()
    {
    
    }
}