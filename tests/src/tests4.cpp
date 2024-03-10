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
#include <cstring>
#include <lilfbtf/test4.h>
#include <any>
#include <variant>
#include <array>
#include <type_traits>
#include <blt/std/logging.h>
#include <blt/std/utility.h>
#include <blt/std/hashmap.h>
#include <blt/std/types.h>
#include <random>

namespace fb
{
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
        ADD, SUB, MUL, DIV, IF, EQUAL_B, EQUAL_N, LESS, GREATER, NOT, AND, OR, VALUE, END
    };
    
    enum class type_category_t
    {
        NUM, BOOL, TERMINAL_NUM, TERMINAL_BOOL, END
    };
    
    std::vector<type_t> combine(const std::vector<type_t>& v1, const std::vector<type_t>& v2)
    {
        std::vector<type_t> types;
        types.reserve(v1.size() + v2.size());
        for (auto v : v1)
            types.push_back(v);
        for (auto v : v2)
            types.push_back(v);
        return types;
    }
    
    std::vector<type_t> VEC_NUM = {type_t::ADD, type_t::SUB, type_t::MUL, type_t::DIV, type_t::IF, type_t::VALUE};
    std::vector<type_t> VEC_BOOL = {type_t::EQUAL_B, type_t::EQUAL_N, type_t::LESS, type_t::GREATER, type_t::NOT, type_t::AND,
                                    type_t::OR};
    
    std::array<std::vector<type_t>, static_cast<int>(type_category_t::END)> categories = {
            VEC_NUM,
            VEC_BOOL,
            {type_t::VALUE},
            {}
    };
    
    const std::array<std::vector<type_category_t>, static_cast<int>(type_t::END)> allowed_arg_types = {
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // ADD
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // SUB
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // MUL
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // DIV
            
            std::vector<type_category_t>{type_category_t::BOOL, type_category_t::NUM, type_category_t::NUM},    // IF
            std::vector<type_category_t>{type_category_t::BOOL, type_category_t::BOOL},                         // EQUAL_B
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // EQUAL_N
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // LESS
            std::vector<type_category_t>{type_category_t::NUM, type_category_t::NUM},                           // GREATER
            std::vector<type_category_t>{type_category_t::BOOL},                                                // NOT
            std::vector<type_category_t>{type_category_t::BOOL, type_category_t::BOOL},                         // AND
            std::vector<type_category_t>{type_category_t::BOOL, type_category_t::BOOL},                         // OR
            
            std::vector<type_category_t>{},                                                                     // VALUE
        
    };
    
    inline type_t random_type(const std::vector<type_t>& allowed_types)
    {
        static std::random_device dev;
        static std::uniform_int_distribution dist(0ul, allowed_types.size() - 1);
        return allowed_types[dist(engine.get())];
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
    
    template<blt::size_t SIZE>
    class any_t_base
    {
        private:
            blt::u8 data[SIZE]{};
        public:
            any_t_base() = default;
            
            template<typename T>
            any_t_base(T t)
            {
                static_assert(std::is_trivially_copyable_v<T> && "Type must be byte copyable");
                static_assert(sizeof(T) <= SIZE && "Size must be less than or equal to internal buffer");
                std::memcpy(data, &t, sizeof(t));
            }
            
            template<typename T>
            any_t_base& operator=(T t)
            {
                static_assert(std::is_trivially_copyable_v<T> && "Type must be byte copyable");
                static_assert(sizeof(T) <= SIZE && "Size must be less than or equal to internal buffer");
                std::memcpy(data, &t, sizeof(t));
            }
            
            template<typename T>
            T any_cast()
            {
                static_assert(std::is_trivially_copyable_v<T> && "Type must be byte copyable");
                static_assert(sizeof(T) <= SIZE && "Size must be less than or equal to internal buffer");
                T t;
                std::memcpy(&t, data, sizeof(T));
                return t;
            }
    };
    
    using VALUE = double;
    using BOOL = bool;
    using any_t = any_t_base<8>;
    
    class func_t;
    class func_any_t;
    
    using func_t_call_t = std::function<void(func_t&, blt::span<any_t>)>;
    using func_any_t_call_t = std::function<void(func_any_t&, blt::span<std::any>)>;
    
    class func_t
    {
        private:
            blt::size_t argc_ = 0;
            const func_t_call_t& func;
        protected:
            any_t value;
        public:
            explicit func_t(blt::size_t argc, const func_t_call_t& func):
                    argc_(argc), func(func)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline any_t getValue() const
            {
                return value;
            }
            
            inline func_t& setValue(any_t val)
            {
                this->value = val;
                return *this;
            }
            
            inline void call(blt::span<any_t> args)
            {
                func(*this, args);
            };
            
            ~func_t() = default;
    };
    
    class func_any_t
    {
        private:
            blt::size_t argc_ = 0;
            const func_any_t_call_t& func;
        protected:
            std::any value;
        public:
            explicit func_any_t(blt::size_t argc, const func_any_t_call_t& func):
                    argc_(argc), func(func)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline std::any getValue() const
            {
                return value;
            }
            
            inline func_any_t& setValue(std::any val)
            {
                this->value = std::move(val);
                return *this;
            }
            
            inline void call(blt::span<std::any> args)
            {
                func(*this, args);
            };
            
            ~func_any_t() = default;
    };
    
    const func_t_call_t add_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() + args[1].any_cast<double>()); };
    const func_t_call_t sub_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() - args[1].any_cast<double>()); };
    const func_t_call_t mul_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() * args[1].any_cast<double>()); };
    const func_t_call_t div_f = [](func_t& us, blt::span<any_t> args) {
        auto dim = args[1].any_cast<double>();
        if (dim == 0)
            us.setValue(0);
        else
            us.setValue(args[0].any_cast<double>() + dim);
    };
    
    const func_t_call_t value_f = [](func_t&, blt::span<any_t>) {};
    const func_t_call_t if_f = [](func_t& us, blt::span<any_t> args) {
        if (args[0].any_cast<bool>())
            us.setValue(args[1].any_cast<double>());
        else
            us.setValue(args[2].any_cast<double>());
    };
    const func_t_call_t equals_b_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<bool>() == args[1].any_cast<bool>()); };
    const func_t_call_t equals_n_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() == args[1].any_cast<double>()); };
    const func_t_call_t less_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() < args[1].any_cast<double>()); };
    const func_t_call_t greater_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() > args[1].any_cast<double>()); };
    const func_t_call_t not_f = [](func_t& us, blt::span<any_t> args) { us.setValue(!args[0].any_cast<bool>()); };
    const func_t_call_t and_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<bool>() && args[1].any_cast<bool>()); };
    const func_t_call_t or_f = [](func_t& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<bool>() || args[1].any_cast<bool>()); };
    
    
    
    
    const func_any_t_call_t add_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<double>(args[0]) + std::any_cast<double>(args[1])); };
    const func_any_t_call_t sub_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<double>(args[0]) - std::any_cast<double>(args[1])); };
    const func_any_t_call_t mul_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<double>(args[0]) * std::any_cast<double>(args[1])); };
    const func_any_t_call_t div_any_f = [](func_any_t& us, blt::span<std::any> args) {
        auto dim = std::any_cast<double>(args[1]);
        if (dim == 0)
            us.setValue(0);
        else
            us.setValue(std::any_cast<double>(args[0]) + dim);
    };
    
    const func_any_t_call_t value_any_f = [](func_any_t&, blt::span<std::any>) {};
    const func_any_t_call_t if_any_f = [](func_any_t& us, blt::span<std::any> args) {
        if (std::any_cast<bool>(args[0]))
            us.setValue(std::any_cast<double>(args[1]));
        else
            us.setValue(std::any_cast<double>(args[2]));
    };
    const func_any_t_call_t equals_b_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<bool>(args[0]) == std::any_cast<bool>(args[1])); };
    const func_any_t_call_t equals_n_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<double>(args[0]) == std::any_cast<double>(args[1])); };
    const func_any_t_call_t less_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<double>(args[0]) < std::any_cast<double>(args[1])); };
    const func_any_t_call_t greater_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<double>(args[0]) > std::any_cast<double>(args[1])); };
    const func_any_t_call_t not_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(!std::any_cast<bool>(args[0])); };
    const func_any_t_call_t and_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<bool>(args[0]) && std::any_cast<bool>(args[1])); };
    const func_any_t_call_t or_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(std::any_cast<bool>(args[0]) || std::any_cast<bool>(args[1])); };
    
    func_t make_type(type_t type)
    {
        switch (type)
        {
            case type_t::ADD:
                return func_t{2, add_f};
            case type_t::SUB:
                return func_t{2, sub_f};
            case type_t::MUL:
                return func_t{2, mul_f};
            case type_t::DIV:
                return func_t{2, div_f};
            case type_t::IF:
                return func_t{3, if_f};
            case type_t::EQUAL_B:
                return func_t{2, equals_b_f};
            case type_t::EQUAL_N:
                return func_t{2, equals_n_f};
            case type_t::LESS:
                return func_t{2, less_f};
            case type_t::GREATER:
                return func_t{2, greater_f};
            case type_t::NOT:
                return func_t{1, not_f};
            case type_t::AND:
                return func_t{2, and_f};
            case type_t::OR:
                return func_t{2, or_f};
            case type_t::VALUE:
                return func_t{0, value_f};
            case type_t::END:
                break;
        }
    }
    
    func_any_t make_type_any(type_t type)
    {
        switch (type)
        {
            case type_t::ADD:
                return func_any_t{2, add_any_f};
            case type_t::SUB:
                return func_any_t{2, sub_any_f};
            case type_t::MUL:
                return func_any_t{2, mul_any_f};
            case type_t::DIV:
                return func_any_t{2, div_any_f};
            case type_t::IF:
                return func_any_t{3, if_any_f};
            case type_t::EQUAL_B:
                return func_any_t{2, equals_b_any_f};
            case type_t::EQUAL_N:
                return func_any_t{2, equals_n_any_f};
            case type_t::LESS:
                return func_any_t{2, less_any_f};
            case type_t::GREATER:
                return func_any_t{2, greater_any_f};
            case type_t::NOT:
                return func_any_t{1, not_any_f};
            case type_t::AND:
                return func_any_t{2, and_any_f};
            case type_t::OR:
                return func_any_t{2, or_any_f};
            case type_t::VALUE:
                return func_any_t{0, value_any_f};
            case type_t::END:
                break;
        }
    }
    
    void test4()
    {
        BLT_INFO(sizeof(std::any));
        BLT_INFO(sizeof(any_t));
    }
    
}