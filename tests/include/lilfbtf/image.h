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

#ifndef GP_IMAGE_TEST_IMAGE_H
#define GP_IMAGE_TEST_IMAGE_H

#include <blt/std/types.h>
#include <array>
#include <optional>
#include "blt/std/assert.h"
#include "blt/std/utility.h"
#include "blt/std/allocator.h"
#include "blt/math/vectors.h"
#include <variant>

class image
{
    private:
        blt::bump_allocator<>& alloc;
        blt::u8* data;
        int _width, _height;
        
        [[nodiscard]] inline blt::size_t data_size() const
        {
            return _width * _height * 3;
        }
    
    public:
        image(int width, int height, blt::bump_allocator<>& alloc): alloc(alloc), _width(width), _height(height)
        {
            data = alloc.allocate<blt::u8>(data_size());
        }
        
        image(const image& copy): alloc(copy.alloc), _width(copy._width), _height(copy._height)
        {
            data = alloc.allocate<blt::u8>(data_size());
            std::memcpy(data, copy.data, data_size());
        }
        
        image(image&& move) noexcept: alloc(move.alloc), _width(move._width), _height(move._height)
        {
            data = move.data;
            move.data = nullptr;
        }
        
        image& operator=(const image& copy) = delete;
        
        image& operator=(image&& move) = delete;
        
        image(const blt::vec3i& v, int width, int height, blt::bump_allocator<>& alloc): alloc(alloc), _width(width), _height(height)
        {
            data = alloc.allocate<blt::u8>(data_size());
            for (blt::size_t i = 0; i < data_size() / 3; i+= 3)
            {
                data[i] = static_cast<blt::u8>(v.x());
                data[i + 1] = static_cast<blt::u8>(v.y());
                data[i + 2] = static_cast<blt::u8>(v.z());
            }
        }
        
        image(int v, int width, int height, blt::bump_allocator<>& alloc): alloc(alloc), _width(width), _height(height)
        {
            data = alloc.allocate<blt::u8>(data_size());
            std::memset(data, static_cast<blt::u8>(v), data_size());
        }
        
        [[nodiscard]] inline blt::u8* getData() const
        {
            return data;
        }
        
        [[nodiscard]] blt::vec3i get(blt::i32 x = 0, blt::i32 y = 0) const
        {
            if (x < 0 || y < 0 || x >= _width || y >= _height)
                return blt::vec3i{0, 0, 0};
            return {data[y * _height + x], data[y * _height + x + 1], data[y * _height + x + 2]};
        }
        
        void set(const blt::vec3i& c, blt::i32 x, blt::i32 y)
        {
            data[y * _height + x] = c.x();
            data[y * _height + x + 1] = c.y();
            data[y * _height + x + 2] = c.z();
        }
        
        ~image()
        {
            alloc.deallocate(data, data_size());
        }
};

#endif //GP_IMAGE_TEST_IMAGE_H
#endif //GP_IMAGE_TEST_IMAGE_H