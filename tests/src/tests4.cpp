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
#include <utility>
#include <variant>
#include <array>
#include <type_traits>
#include <blt/std/logging.h>
#include <blt/std/utility.h>
#include <blt/std/hashmap.h>
#include <blt/std/types.h>
#include <random>
#include <thread>
#include <stack>
#include "blt/profiling/profiler_v2.h"
#include "blt/std/allocator.h"
#include "blt/std/format.h"
#include "blt/std/system.h"
#include "lilfbtf/tree.h"


namespace fb
{
    template<typename T>
    inline bool once()
    {
        static bool b = true;
        return std::exchange(b, false);
    }
    
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
        NUM, BOOL, NON_TERMINALS, TERMINAL_NUM, TERMINAL_BOOL, END
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
    
    std::vector<type_t> VEC_NUM = {type_t::ADD, type_t::SUB, type_t::MUL, type_t::DIV, type_t::IF};
    std::vector<type_t> VEC_NUM_TERMINAL = {type_t::ADD, type_t::SUB, type_t::MUL, type_t::DIV, type_t::IF, type_t::VALUE};
    std::vector<type_t> VEC_BOOL = {type_t::EQUAL_B, type_t::EQUAL_N, type_t::LESS, type_t::GREATER, type_t::NOT, type_t::AND,
                                    type_t::OR};
    std::vector<type_t> VEC_NON_TERMINAL = {type_t::ADD, type_t::SUB, type_t::MUL, type_t::DIV, type_t::IF, type_t::EQUAL_B,
                                            type_t::EQUAL_N, type_t::LESS, type_t::GREATER, type_t::NOT, type_t::AND,
                                            type_t::OR};
    
    std::array<std::vector<type_t>, static_cast<int>(type_category_t::END)> categories = {
            VEC_NUM,
            VEC_BOOL,
            VEC_NON_TERMINAL,
            VEC_NUM_TERMINAL,
            VEC_BOOL
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
        std::uniform_int_distribution dist(0ul, allowed_types.size() - 1);
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
    
    class any_t_variant
    {
        private:
            static constexpr auto SIZE = sizeof(std::any);
            using array_t = std::array<blt::u8, SIZE>;
            std::variant<std::any, array_t> data;
        public:
            any_t_variant() = default;
            
            template<typename T>
            any_t_variant(T t)
            {
                if constexpr (sizeof(T) <= SIZE && std::is_trivially_copyable_v<T>)
                {
                    data = array_t{};
                    std::memcpy(std::get<array_t>(data).data(), &t, sizeof(t));
                } else
                    data = t;
            }
            
            template<typename T>
            any_t_variant& operator=(T t)
            {
                if constexpr (sizeof(T) <= SIZE && std::is_trivially_copyable_v<T>)
                {
                    if (!std::holds_alternative<std::array<blt::u8, SIZE>>(data))
                        data = std::array<blt::u8, SIZE>{};
                    std::memcpy(std::get<array_t>(data).data(), &t, sizeof(t));
                } else
                    data = t;
                return *this;
            }
            
            template<typename T>
            T any_cast()
            {
                if constexpr (sizeof(T) <= SIZE && std::is_trivially_copyable_v<T>)
                {
                    if (std::holds_alternative<array_t>(data))
                    {
                        T t;
                        std::memcpy(&t, std::get<array_t>(data).data(), sizeof(T));
                        return t;
                    }
                }
                return std::any_cast<T>(std::get<std::any>(data));
            }
    };
    
    class any_t_union
    {
        private:
            static constexpr auto SIZE = sizeof(std::any);
            
            union variant_t
            {
                constexpr variant_t()
                {}
                
                blt::u8 data[SIZE]{};
                std::any any;
                
                ~variant_t()
                {}
            };
            
            variant_t variant;
            bool has_any = false;
        public:
            any_t_union() = default;
            
            any_t_union(const any_t_union& copy)
            {
                if (copy.has_any)
                {
                    variant.any = copy.variant.any;
                    has_any = true;
                } else
                {
                    std::memcpy(variant.data, copy.variant.data, SIZE);
                }
            }
            
            any_t_union(any_t_union&& move) noexcept
            {
                if (move.has_any)
                {
                    variant.any = std::move(move.variant.any);
                    has_any = true;
                } else
                {
                    std::memcpy(variant.data, move.variant.data, SIZE);
                }
            }
            
            ~any_t_union()
            {
                if (has_any)
                    variant.any.~any();
            }
            
            template<typename T>
            any_t_union(T t)
            {
                if constexpr (sizeof(T) <= SIZE && std::is_trivially_copyable_v<T>)
                {
                    std::memcpy(variant.data, &t, sizeof(t));
                } else
                {
                    variant.any = t;
                    has_any = true;
                }
            }
            
            any_t_union& operator=(const any_t_union& copy)
            {
                if (has_any)
                    variant.any.~any();
                if (copy.has_any)
                {
                    variant.any = copy.variant.any;
                    has_any = true;
                } else
                {
                    std::memcpy(variant.data, copy.variant.data, SIZE);
                    has_any = false;
                }
                return *this;
            }
            
            any_t_union& operator=(any_t_union&& move) noexcept
            {
                if (has_any)
                    variant.any.~any();
                if (move.has_any)
                {
                    variant.any = std::move(move.variant.any);
                    has_any = true;
                } else
                {
                    std::memcpy(variant.data, move.variant.data, SIZE);
                    has_any = false;
                }
                return *this;
            }
            
            template<typename T>
            any_t_union& operator=(T t)
            {
                if (has_any)
                    variant.any.~any();
                if constexpr (sizeof(T) <= SIZE && std::is_trivially_copyable_v<T>)
                {
                    std::memcpy(variant.data, &t, sizeof(t));
                    has_any = false;
                } else
                {
                    variant.any = t;
                    has_any = true;
                }
                return *this;
            }
            
            template<typename T>
            T any_cast()
            {
                if constexpr (sizeof(T) <= SIZE && std::is_trivially_copyable_v<T>)
                {
                    if (!has_any)
                    {
                        T t;
                        std::memcpy(&t, variant.data, sizeof(T));
                        return t;
                    }
                }
                return std::any_cast<T>(variant.any);
            }
    };
    
    using any_t = any_t_base<8>;
    //using any_t = any_t_variant;
    
    class func_t_old;
    
    class func_variant_t;
    
    class func_union_t;
    
    class func_any_t;
    
    using func_t_call_t_old = std::function<void(func_t_old&, blt::span<any_t>)>;
    using func_variant_t_call_t = std::function<void(func_variant_t&, blt::span<any_t_variant>)>;
    using func_union_t_call_t = std::function<void(func_union_t&, blt::span<any_t_union>)>;
    using func_any_t_call_t = std::function<void(func_any_t&, blt::span<std::any>)>;
    
    class func_t_old
    {
        private:
            blt::size_t argc_ = 0;
            const func_t_call_t_old& func;
        protected:
            any_t value;
        public:
            explicit func_t_old(blt::size_t argc, const func_t_call_t_old& func):
                    argc_(argc), func(func)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline any_t getValue() const
            {
                return value;
            }
            
            inline func_t_old& setValue(any_t val)
            {
                this->value = val;
                return *this;
            }
            
            inline void call(blt::span<any_t> args)
            {
                func(*this, args);
            };
            
            ~func_t_old() = default;
    };
    
    class func_variant_t
    {
        private:
            blt::size_t argc_ = 0;
            const func_variant_t_call_t& func;
        protected:
            any_t_variant value;
        public:
            explicit func_variant_t(blt::size_t argc, const func_variant_t_call_t& func):
                    argc_(argc), func(func)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline any_t_variant getValue() const
            {
                return value;
            }
            
            inline func_variant_t& setValue(any_t_variant val)
            {
                this->value = std::move(val);
                return *this;
            }
            
            inline void call(blt::span<any_t_variant> args)
            {
                func(*this, args);
            };
            
            ~func_variant_t() = default;
    };
    
    class func_union_t
    {
        private:
            blt::size_t argc_ = 0;
            const func_union_t_call_t& func;
        protected:
            any_t_union value;
        public:
            explicit func_union_t(blt::size_t argc, const func_union_t_call_t& func):
                    argc_(argc), func(func)
            {}
            
            [[nodiscard]] inline blt::size_t argc() const
            { return argc_; }
            
            [[nodiscard]] inline any_t_union getValue() const
            {
                return value;
            }
            
            inline func_union_t& setValue(any_t_union val)
            {
                this->value = val;
                return *this;
            }
            
            inline void call(blt::span<any_t_union> args)
            {
                func(*this, args);
            };
            
            ~func_union_t() = default;
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
    
    const func_t_call_t_old add_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() + args[1].any_cast<double>()); };
    const func_t_call_t_old sub_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() - args[1].any_cast<double>()); };
    const func_t_call_t_old mul_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() * args[1].any_cast<double>()); };
    const func_t_call_t_old div_f = [](func_t_old& us, blt::span<any_t> args) {
        auto dim = args[1].any_cast<double>();
        if (dim == 0)
            us.setValue(0);
        else
            us.setValue(args[0].any_cast<double>() + dim);
    };
    
    const func_t_call_t_old value_f = [](func_t_old&, blt::span<any_t>) {};
    const func_t_call_t_old if_f = [](func_t_old& us, blt::span<any_t> args) {
        if (args[0].any_cast<bool>())
            us.setValue(args[1].any_cast<double>());
        else
            us.setValue(args[2].any_cast<double>());
    };
    const func_t_call_t_old equals_b_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<bool>() == args[1].any_cast<bool>()); };
    const func_t_call_t_old equals_n_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() == args[1].any_cast<double>()); };
    const func_t_call_t_old less_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() < args[1].any_cast<double>()); };
    const func_t_call_t_old greater_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<double>() > args[1].any_cast<double>()); };
    const func_t_call_t_old not_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(!args[0].any_cast<bool>()); };
    const func_t_call_t_old and_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<bool>() && args[1].any_cast<bool>()); };
    const func_t_call_t_old or_f = [](func_t_old& us, blt::span<any_t> args) { us.setValue(args[0].any_cast<bool>() || args[1].any_cast<bool>()); };
    
    
    const func_variant_t_call_t add_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<double>() + args[1].any_cast<double>());
    };
    const func_variant_t_call_t sub_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<double>() - args[1].any_cast<double>());
    };
    const func_variant_t_call_t mul_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<double>() * args[1].any_cast<double>());
    };
    const func_variant_t_call_t div_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        auto dim = args[1].any_cast<double>();
        if (dim == 0)
            us.setValue(0);
        else
            us.setValue(args[0].any_cast<double>() + dim);
    };
    
    const func_variant_t_call_t value_variant_f = [](func_variant_t&, blt::span<any_t_variant>) {};
    const func_variant_t_call_t if_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        if (args[0].any_cast<bool>())
            us.setValue(args[1].any_cast<double>());
        else
            us.setValue(args[2].any_cast<double>());
    };
    const func_variant_t_call_t equals_b_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<bool>() == args[1].any_cast<bool>());
    };
    const func_variant_t_call_t equals_n_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<double>() == args[1].any_cast<double>());
    };
    const func_variant_t_call_t less_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<double>() < args[1].any_cast<double>());
    };
    const func_variant_t_call_t greater_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<double>() > args[1].any_cast<double>());
    };
    const func_variant_t_call_t not_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) { us.setValue(!args[0].any_cast<bool>()); };
    const func_variant_t_call_t and_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<bool>() && args[1].any_cast<bool>());
    };
    const func_variant_t_call_t or_variant_f = [](func_variant_t& us, blt::span<any_t_variant> args) {
        us.setValue(args[0].any_cast<bool>() || args[1].any_cast<bool>());
    };
    
    
    const func_union_t_call_t add_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<double>() + args[1].any_cast<double>());
    };
    const func_union_t_call_t sub_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<double>() - args[1].any_cast<double>());
    };
    const func_union_t_call_t mul_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<double>() * args[1].any_cast<double>());
    };
    const func_union_t_call_t div_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        auto dim = args[1].any_cast<double>();
        if (dim == 0)
            us.setValue(0);
        else
            us.setValue(args[0].any_cast<double>() + dim);
    };
    
    const func_union_t_call_t value_union_f = [](func_union_t&, blt::span<any_t_union>) {};
    const func_union_t_call_t if_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        if (args[0].any_cast<bool>())
            us.setValue(args[1].any_cast<double>());
        else
            us.setValue(args[2].any_cast<double>());
    };
    const func_union_t_call_t equals_b_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<bool>() == args[1].any_cast<bool>());
    };
    const func_union_t_call_t equals_n_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<double>() == args[1].any_cast<double>());
    };
    const func_union_t_call_t less_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<double>() < args[1].any_cast<double>());
    };
    const func_union_t_call_t greater_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<double>() > args[1].any_cast<double>());
    };
    const func_union_t_call_t not_union_f = [](func_union_t& us, blt::span<any_t_union> args) { us.setValue(!args[0].any_cast<bool>()); };
    const func_union_t_call_t and_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<bool>() && args[1].any_cast<bool>());
    };
    const func_union_t_call_t or_union_f = [](func_union_t& us, blt::span<any_t_union> args) {
        us.setValue(args[0].any_cast<bool>() || args[1].any_cast<bool>());
    };
    
    
    const func_any_t_call_t add_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<double>(args[0]) + std::any_cast<double>(args[1]));
    };
    const func_any_t_call_t sub_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<double>(args[0]) - std::any_cast<double>(args[1]));
    };
    const func_any_t_call_t mul_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<double>(args[0]) * std::any_cast<double>(args[1]));
    };
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
    const func_any_t_call_t equals_b_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<bool>(args[0]) == std::any_cast<bool>(args[1]));
    };
    const func_any_t_call_t equals_n_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<double>(args[0]) == std::any_cast<double>(args[1]));
    };
    const func_any_t_call_t less_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<double>(args[0]) < std::any_cast<double>(args[1]));
    };
    const func_any_t_call_t greater_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<double>(args[0]) > std::any_cast<double>(args[1]));
    };
    const func_any_t_call_t not_any_f = [](func_any_t& us, blt::span<std::any> args) { us.setValue(!std::any_cast<bool>(args[0])); };
    const func_any_t_call_t and_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<bool>(args[0]) && std::any_cast<bool>(args[1]));
    };
    const func_any_t_call_t or_any_f = [](func_any_t& us, blt::span<std::any> args) {
        us.setValue(std::any_cast<bool>(args[0]) || std::any_cast<bool>(args[1]));
    };
    
    
    func_t_old make_type(type_t type)
    {
        switch (type)
        {
            case type_t::ADD:
                return func_t_old{2, add_f};
            case type_t::SUB:
                return func_t_old{2, sub_f};
            case type_t::MUL:
                return func_t_old{2, mul_f};
            case type_t::DIV:
                return func_t_old{2, div_f};
            case type_t::IF:
                return func_t_old{3, if_f};
            case type_t::EQUAL_B:
                return func_t_old{2, equals_b_f};
            case type_t::EQUAL_N:
                return func_t_old{2, equals_n_f};
            case type_t::LESS:
                return func_t_old{2, less_f};
            case type_t::GREATER:
                return func_t_old{2, greater_f};
            case type_t::NOT:
                return func_t_old{1, not_f};
            case type_t::AND:
                return func_t_old{2, and_f};
            case type_t::OR:
                return func_t_old{2, or_f};
            case type_t::VALUE:
                return func_t_old{0, value_f}.setValue(random_value());
            case type_t::END:
                break;
        }
        BLT_WARN("How did we get here? input %d", static_cast<int>(type));
        return func_t_old{0, value_f}.setValue(random_value());
    }
    
    func_variant_t make_type_variant(type_t type)
    {
        switch (type)
        {
            case type_t::ADD:
                return func_variant_t{2, add_variant_f};
            case type_t::SUB:
                return func_variant_t{2, sub_variant_f};
            case type_t::MUL:
                return func_variant_t{2, mul_variant_f};
            case type_t::DIV:
                return func_variant_t{2, div_variant_f};
            case type_t::IF:
                return func_variant_t{3, if_variant_f};
            case type_t::EQUAL_B:
                return func_variant_t{2, equals_b_variant_f};
            case type_t::EQUAL_N:
                return func_variant_t{2, equals_n_variant_f};
            case type_t::LESS:
                return func_variant_t{2, less_variant_f};
            case type_t::GREATER:
                return func_variant_t{2, greater_variant_f};
            case type_t::NOT:
                return func_variant_t{1, not_variant_f};
            case type_t::AND:
                return func_variant_t{2, and_variant_f};
            case type_t::OR:
                return func_variant_t{2, or_variant_f};
            case type_t::VALUE:
                return func_variant_t{0, value_variant_f}.setValue(random_value());
            case type_t::END:
                break;
        }
        BLT_WARN("How did we get here? input %d", static_cast<int>(type));
        return func_variant_t{0, value_variant_f}.setValue(random_value());
    }
    
    func_union_t make_type_union(type_t type)
    {
        switch (type)
        {
            case type_t::ADD:
                return func_union_t{2, add_union_f};
            case type_t::SUB:
                return func_union_t{2, sub_union_f};
            case type_t::MUL:
                return func_union_t{2, mul_union_f};
            case type_t::DIV:
                return func_union_t{2, div_union_f};
            case type_t::IF:
                return func_union_t{3, if_union_f};
            case type_t::EQUAL_B:
                return func_union_t{2, equals_b_union_f};
            case type_t::EQUAL_N:
                return func_union_t{2, equals_n_union_f};
            case type_t::LESS:
                return func_union_t{2, less_union_f};
            case type_t::GREATER:
                return func_union_t{2, greater_union_f};
            case type_t::NOT:
                return func_union_t{1, not_union_f};
            case type_t::AND:
                return func_union_t{2, and_union_f};
            case type_t::OR:
                return func_union_t{2, or_union_f};
            case type_t::VALUE:
                return func_union_t{0, value_union_f}.setValue(random_value());
            case type_t::END:
                break;
        }
        BLT_WARN("How did we get here? input %d", static_cast<int>(type));
        return func_union_t{0, value_union_f}.setValue(random_value());
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
                return func_any_t{0, value_any_f}.setValue(random_value());
            case type_t::END:
                break;
        }
        BLT_WARN("How did we get here? input %d", static_cast<int>(type));
        return func_any_t{0, value_any_f}.setValue(random_value());
    }
    
    blt::bump_allocator<blt::BLT_2MB_SIZE, false> alloc_2;
    
    class tree_any
    {
        private:
            struct node_t
            {
                func_t_old type;
                type_t enum_type;
                node_t** children = nullptr;
                
                explicit node_t(type_t type): type(make_type(type)), enum_type(type)
                {
                    children = alloc_2.emplace_many<node_t*>(this->type.argc());
                    for (blt::size_t i = 0; i < this->type.argc(); i++)
                        children[i] = nullptr;
                }
                
                void evaluate()
                {
                    if (type.argc() > 0)
                    {
                        any_t d[3]{};
                        for (blt::size_t i = 0; i < type.argc(); i++)
                            d[i] = children[i]->type.getValue();
                        
                        type.call(blt::span{d, type.argc()});
                    } else
                        type.call({});
                }
                
                double evaluate_tree()
                {
                    std::stack<node_t*> nodes;
                    std::stack<node_t*> node_stack;
                    
                    nodes.push(this);
                    
                    while (!nodes.empty())
                    {
                        auto* top = nodes.top();
                        node_stack.push(top);
                        nodes.pop();
                        for (blt::size_t i = 0; i < top->type.argc(); i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    return type.getValue().any_cast<double>();
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type.argc(); i++)
                    {
                        alloc_2.destroy(children[i]);
                        alloc_2.deallocate(children[i]);
                    }
                    alloc_2.deallocate(children, type.argc());
                }
            };
            
            node_t* root = nullptr;
        public:
            tree_any()
            {
                if (once<tree_any>())
                    BLT_INFO(sizeof(node_t));
            };
            
            void create(blt::u64 size)
            {
                root = alloc_2.template emplace<node_t>(random_type(VEC_NON_TERMINAL));
                std::stack<std::pair<node_t*, blt::size_t>> stack;
                stack.emplace(root, 0);
                while (!stack.empty())
                {
                    auto top = stack.top();
                    auto* node = top.first;
                    auto depth = top.second;
                    stack.pop();
                    auto& allowed_types = allowed_arg_types[static_cast<int>(node->enum_type)];
                    for (blt::size_t i = 0; i < node->type.argc(); i++)
                    {
                        auto type_category = allowed_types[i];
                        if (depth >= size)
                        {
                            if (type_category == type_category_t::BOOL)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::GREATER);
                                node->children[i]->children[0] = alloc_2.template emplace<node_t>(type_t::VALUE);
                                node->children[i]->children[1] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            } else
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            }
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        else
                        {
                            if (type_category == type_category_t::NUM)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(
                                        random_type(categories[static_cast<int>(type_category_t::TERMINAL_NUM)]));
                            } else
                                node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        }
                        if (depth < size)
                            stack.emplace(node->children[i], depth + 1);
                    }
                }
            }
            
            double evaluate()
            {
                return root->evaluate_tree();
            }
            
            ~tree_any()
            {
                BLT_START_INTERVAL("Tree Destruction", "any_t tree");
                alloc_2.destroy(root);
                alloc_2.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", "any_t tree");
            }
    };
    
    class tree_any_variant
    {
        private:
            struct node_t
            {
                func_variant_t type;
                type_t enum_type;
                node_t** children = nullptr;
                
                explicit node_t(type_t type): type(make_type_variant(type)), enum_type(type)
                {
                    children = alloc_2.emplace_many<node_t*>(this->type.argc());
                    for (blt::size_t i = 0; i < this->type.argc(); i++)
                        children[i] = nullptr;
                }
                
                void evaluate()
                {
                    if (type.argc() > 0)
                    {
                        any_t_variant d[3]{};
                        for (blt::size_t i = 0; i < type.argc(); i++)
                            d[i] = children[i]->type.getValue();
                        
                        type.call(blt::span{d, type.argc()});
                    } else
                        type.call({});
                }
                
                double evaluate_tree()
                {
                    std::stack<node_t*> nodes;
                    std::stack<node_t*> node_stack;
                    
                    nodes.push(this);
                    
                    while (!nodes.empty())
                    {
                        auto* top = nodes.top();
                        node_stack.push(top);
                        nodes.pop();
                        for (blt::size_t i = 0; i < top->type.argc(); i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    return type.getValue().any_cast<double>();
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type.argc(); i++)
                    {
                        alloc_2.destroy(children[i]);
                        alloc_2.deallocate(children[i]);
                    }
                    alloc_2.deallocate(children, type.argc());
                }
            };
            
            node_t* root = nullptr;
        public:
            tree_any_variant()
            {
                if (once<tree_any_variant>())
                    BLT_INFO(sizeof(node_t));
            };
            
            void create(blt::u64 size)
            {
                root = alloc_2.template emplace<node_t>(random_type(VEC_NON_TERMINAL));
                std::stack<std::pair<node_t*, blt::size_t>> stack;
                stack.emplace(root, 0);
                while (!stack.empty())
                {
                    auto top = stack.top();
                    auto* node = top.first;
                    auto depth = top.second;
                    stack.pop();
                    auto& allowed_types = allowed_arg_types[static_cast<int>(node->enum_type)];
                    for (blt::size_t i = 0; i < node->type.argc(); i++)
                    {
                        auto type_category = allowed_types[i];
                        if (depth >= size)
                        {
                            if (type_category == type_category_t::BOOL)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::GREATER);
                                node->children[i]->children[0] = alloc_2.template emplace<node_t>(type_t::VALUE);
                                node->children[i]->children[1] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            } else
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            }
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        else
                        {
                            if (type_category == type_category_t::NUM)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(
                                        random_type(categories[static_cast<int>(type_category_t::TERMINAL_NUM)]));
                            } else
                                node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        }
                        if (depth < size)
                            stack.emplace(node->children[i], depth + 1);
                    }
                }
            }
            
            double evaluate()
            {
                return root->evaluate_tree();
            }
            
            ~tree_any_variant()
            {
                BLT_START_INTERVAL("Tree Destruction", "any_t_variant tree");
                alloc_2.destroy(root);
                alloc_2.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", "any_t_variant tree");
            }
    };
    
    class tree_any_union
    {
        private:
            struct node_t
            {
                func_union_t type;
                type_t enum_type;
                node_t** children = nullptr;
                
                explicit node_t(type_t type): type(make_type_union(type)), enum_type(type)
                {
                    children = alloc_2.emplace_many<node_t*>(this->type.argc());
                    for (blt::size_t i = 0; i < this->type.argc(); i++)
                        children[i] = nullptr;
                }
                
                void evaluate()
                {
                    if (type.argc() > 0)
                    {
                        any_t_union d[3]{};
                        for (blt::size_t i = 0; i < type.argc(); i++)
                            d[i] = children[i]->type.getValue();
                        
                        type.call(blt::span{d, type.argc()});
                    } else
                        type.call({});
                }
                
                double evaluate_tree()
                {
                    std::stack<node_t*> nodes;
                    std::stack<node_t*> node_stack;
                    
                    nodes.push(this);
                    
                    while (!nodes.empty())
                    {
                        auto* top = nodes.top();
                        node_stack.push(top);
                        nodes.pop();
                        for (blt::size_t i = 0; i < top->type.argc(); i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    return type.getValue().any_cast<double>();
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type.argc(); i++)
                    {
                        alloc_2.destroy(children[i]);
                        alloc_2.deallocate(children[i]);
                    }
                    alloc_2.deallocate(children, type.argc());
                }
            };
            
            node_t* root = nullptr;
        public:
            tree_any_union()
            {
                if (once<tree_any_union>())
                    BLT_INFO(sizeof(node_t));
            };
            
            void create(blt::u64 size)
            {
                root = alloc_2.template emplace<node_t>(random_type(VEC_NON_TERMINAL));
                std::stack<std::pair<node_t*, blt::size_t>> stack;
                stack.emplace(root, 0);
                while (!stack.empty())
                {
                    auto top = stack.top();
                    auto* node = top.first;
                    auto depth = top.second;
                    stack.pop();
                    auto& allowed_types = allowed_arg_types[static_cast<int>(node->enum_type)];
                    for (blt::size_t i = 0; i < node->type.argc(); i++)
                    {
                        auto type_category = allowed_types[i];
                        if (depth >= size)
                        {
                            if (type_category == type_category_t::BOOL)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::GREATER);
                                node->children[i]->children[0] = alloc_2.template emplace<node_t>(type_t::VALUE);
                                node->children[i]->children[1] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            } else
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            }
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        else
                        {
                            if (type_category == type_category_t::NUM)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(
                                        random_type(categories[static_cast<int>(type_category_t::TERMINAL_NUM)]));
                            } else
                                node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        }
                        if (depth < size)
                            stack.emplace(node->children[i], depth + 1);
                    }
                }
            }
            
            double evaluate()
            {
                return root->evaluate_tree();
            }
            
            ~tree_any_union()
            {
                BLT_START_INTERVAL("Tree Destruction", "any_t_union tree");
                alloc_2.destroy(root);
                alloc_2.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", "any_t_union tree");
            }
    };
    
    class tree_std_any
    {
        private:
            struct node_t
            {
                func_any_t type;
                type_t enum_type;
                node_t** children = nullptr;
                
                explicit node_t(type_t type): type(make_type_any(type)), enum_type(type)
                {
                    children = alloc_2.emplace_many<node_t*>(this->type.argc());
                    for (blt::size_t i = 0; i < this->type.argc(); i++)
                        children[i] = nullptr;
                }
                
                void evaluate()
                {
                    if (type.argc() > 0)
                    {
                        std::any d[3]{};
                        for (blt::size_t i = 0; i < type.argc(); i++)
                            d[i] = children[i]->type.getValue();
                        
                        type.call(blt::span{d, type.argc()});
                    } else
                        type.call({});
                }
                
                double evaluate_tree()
                {
                    std::stack<node_t*> nodes;
                    std::stack<node_t*> node_stack;
                    
                    nodes.push(this);
                    
                    while (!nodes.empty())
                    {
                        auto* top = nodes.top();
                        node_stack.push(top);
                        nodes.pop();
                        for (blt::size_t i = 0; i < top->type.argc(); i++)
                            nodes.push(top->children[i]);
                    }
                    
                    while (!node_stack.empty())
                    {
                        node_stack.top()->evaluate();
                        node_stack.pop();
                    }
                    if (type.getValue().type() == typeid(double))
                        return std::any_cast<double>(type.getValue());
                    else
                        return std::any_cast<bool>(type.getValue());
                }
                
                ~node_t()
                {
                    for (blt::size_t i = 0; i < type.argc(); i++)
                    {
                        alloc_2.destroy(children[i]);
                        alloc_2.deallocate(children[i]);
                    }
                    alloc_2.deallocate(children, type.argc());
                }
            };
            
            node_t* root = nullptr;
        public:
            tree_std_any()
            {
                if (once<tree_std_any>())
                    BLT_INFO(sizeof(node_t));
            };
            
            void create(blt::u64 size)
            {
                root = alloc_2.template emplace<node_t>(random_type(VEC_NON_TERMINAL));
                std::stack<std::pair<node_t*, blt::size_t>> stack;
                stack.emplace(root, 0);
                while (!stack.empty())
                {
                    auto top = stack.top();
                    auto* node = top.first;
                    auto depth = top.second;
                    stack.pop();
                    auto& allowed_types = allowed_arg_types[static_cast<int>(node->enum_type)];
                    for (blt::size_t i = 0; i < node->type.argc(); i++)
                    {
                        auto type_category = allowed_types[i];
                        if (depth >= size)
                        {
                            if (type_category == type_category_t::BOOL)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::GREATER);
                                node->children[i]->children[0] = alloc_2.template emplace<node_t>(type_t::VALUE);
                                node->children[i]->children[1] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            } else
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(type_t::VALUE);
                            }
                            continue;
                        }
                        if (choice())
                            node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        else
                        {
                            if (type_category == type_category_t::NUM)
                            {
                                node->children[i] = alloc_2.template emplace<node_t>(
                                        random_type(categories[static_cast<int>(type_category_t::TERMINAL_NUM)]));
                            } else
                                node->children[i] = alloc_2.template emplace<node_t>(random_type(categories[static_cast<int>(type_category)]));
                        }
                        if (depth < size)
                            stack.emplace(node->children[i], depth + 1);
                    }
                }
            }
            
            double evaluate()
            {
                return root->evaluate_tree();
            }
            
            ~tree_std_any()
            {
                BLT_START_INTERVAL("Tree Destruction", "std::any tree");
                alloc_2.destroy(root);
                alloc_2.deallocate(root);
                BLT_END_INTERVAL("Tree Destruction", "std::any tree");
            }
    };
    
    constexpr auto size = 128;
    constexpr auto tree_size = 17;
    
    void run_any_t()
    {
        alloc_2.resetStats();
        engine.reset();
        tree_any love[size];
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Construction", "any_t tree");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", "any_t tree");
        BLT_INFO("Construction any_t finished");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Evaluation", "any_t tree");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", "any_t tree");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_INFO("Peak bytes for any_t: %s, blocks: %ld", blt::string::fromBytes(alloc_2.getStats().getPeakBytes()).c_str(),
                 alloc_2.getStats().getPeakBlocks());
    }
    
    void run_any_t_variant()
    {
        alloc_2.resetStats();
        engine.reset();
        tree_any_variant love[size];
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Construction", "any_t_variant tree");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", "any_t_variant tree");
        BLT_INFO("Construction any_t_variant finished");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Evaluation", "any_t_variant tree");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", "any_t_variant tree");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_INFO("Peak bytes for any_t_variant: %s, blocks: %ld", blt::string::fromBytes(alloc_2.getStats().getPeakBytes()).c_str(),
                 alloc_2.getStats().getPeakBlocks());
    }
    
    void run_any_t_union()
    {
        alloc_2.resetStats();
        engine.reset();
        tree_any_union love[size];
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Construction", "any_t_union tree");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", "any_t_union tree");
        BLT_INFO("Construction any_t_union finished");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Evaluation", "any_t_union tree");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", "any_t_union tree");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_INFO("Peak bytes for any_t_union: %s, blocks: %ld", blt::string::fromBytes(alloc_2.getStats().getPeakBytes()).c_str(),
                 alloc_2.getStats().getPeakBlocks());
    }
    
    void run_std_any_t()
    {
        alloc_2.resetStats();
        engine.reset();
        tree_std_any love[size];
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Construction", "std::any tree");
        for (auto& i : love)
            i.create(tree_size);
        BLT_END_INTERVAL("Tree Construction", "std::any tree");
        BLT_INFO("Construction std::any finished");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_START_INTERVAL("Tree Evaluation", "std::any tree");
        for (auto& i : love)
            blt::black_box(i.evaluate());
        BLT_END_INTERVAL("Tree Evaluation", "std::any tree");
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_INFO("Peak bytes for std::any: %s, blocks: %ld", blt::string::fromBytes(alloc_2.getStats().getPeakBytes()).c_str(),
                 alloc_2.getStats().getPeakBlocks());
    }
    
    void test4()
    {
        BLT_INFO(sizeof(std::any));
        BLT_INFO(sizeof(any_t));
        BLT_INFO(sizeof(any_t_variant));
        BLT_INFO(sizeof(any_t_union));
        
        run_any_t();
        //run_any_t_variant();
        //run_any_t_union();
        //run_std_any_t();
        
        for (int i = 0; i < 500; i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        {
            auto v = blt::system::get_memory_process();
            BLT_DEBUG("Currently %ld bytes aka %s in memory", v.resident, blt::string::fromBytes(v.resident).c_str());
        }
        BLT_INFO("Current bytes: %s, blocks: %ld", blt::string::fromBytes(alloc_2.getStats().getAllocatedBytes()).c_str(),
                 alloc_2.getStats().getAllocatedBlocks());
    }
}