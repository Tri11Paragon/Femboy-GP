#include <iostream>
#include <utility>
#include <memory>
#include <blt/std/logging.h>
#include <blt/parse/argparse.h>
#include <lilfbtf/test2.h>
#include <lilfbtf/test3.h>
#include <lilfbtf/test4.h>
#include "blt/profiling/profiler_v2.h"
#include "lilfbtf/test5.h"
#include <lilfbtf/tree.h>
#include <lilfbtf/type.h>

struct data
{
    float f;
    int i;
    char c;
};

struct pixel
{
    blt::size_t x, y;
};

const blt::size_t image_width = 128, image_height = 128;

const fb::func_t_call_t add_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() + args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t sub_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() - args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t mul_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() * args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t div_f = [](const fb::detail::func_t_arguments& args) {
    auto dim = args.arguments[1]->value().any_cast<blt::u8>();
    if (dim == 0)
        args.self.setValue(0);
    else
        args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() + dim);
};

const fb::func_t_call_t empty_f = [](const fb::detail::func_t_arguments&) {};
const fb::func_t_call_t coord_x_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.extra_args.any_cast<pixel>().x);
};
const fb::func_t_call_t coord_y_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.extra_args.any_cast<pixel>().y);
};
const fb::func_t_init_t value_init_f = [](fb::func_t& self) {
    self.setValue(fb::random_value());
};
const fb::func_t_init_t bool_init_f = [](fb::func_t& self) {
    self.setValue(fb::choice());
};
const fb::func_t_call_t if_f = [](const fb::detail::func_t_arguments& args) {
    if (args.arguments[0]->value().any_cast<bool>())
        args.self.setValue(args.arguments[1]->value().any_cast<blt::u8>());
    else
        args.self.setValue(args.arguments[2]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t equals_b_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<bool>() == args.arguments[1]->value().any_cast<bool>());
};
const fb::func_t_call_t equals_n_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() == args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t less_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() < args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t greater_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() > args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t not_f = [](const fb::detail::func_t_arguments& args) { args.self.setValue(!args.arguments[0]->value().any_cast<bool>()); };
const fb::func_t_call_t and_b_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<bool>() && args.arguments[1]->value().any_cast<bool>());
};
const fb::func_t_call_t or_b_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<bool>() || args.arguments[1]->value().any_cast<bool>());
};

const fb::func_t_call_t and_n_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() & args.arguments[1]->value().any_cast<blt::u8>());
};
const fb::func_t_call_t or_n_f = [](const fb::detail::func_t_arguments& args) {
    args.self.setValue(args.arguments[0]->value().any_cast<blt::u8>() | args.arguments[1]->value().any_cast<blt::u8>());
};

int main(int argc, const char** argv)
{
    size_t size = 32;
    size_t remaining_bytes = size;
    size_t offset = 0;
    void* void_ptr = nullptr;
    char* new_ptr = nullptr;
    char* buffer = new char[size];
    
    remaining_bytes = size - offset;
    std::cout << static_cast<void*>(buffer) << " ' " << remaining_bytes << std::endl;
    void_ptr = reinterpret_cast<void*>(&buffer[offset]);
    new_ptr = static_cast<char*>(std::align(alignof(char), sizeof(char), void_ptr, remaining_bytes));
    std::cout << static_cast<void*>(new_ptr) << " : " << remaining_bytes << " | " << (buffer - new_ptr + 1) << std::endl;
    offset += (buffer - new_ptr + 1);
    
    remaining_bytes = size - offset;
    void_ptr = reinterpret_cast<void*>(&buffer[offset]);
    new_ptr = static_cast<char*>(std::align(alignof(data), sizeof(data), void_ptr, remaining_bytes));
    std::cout << static_cast<void*>(new_ptr) << " : " << remaining_bytes << " | " << (buffer - new_ptr + sizeof(data)) << std::endl;
    offset += (buffer - new_ptr + sizeof(data));
    
    delete[](buffer);
    
    std::cout << "Hello, World!" << std::endl;
    
    blt::arg_parse parser;
    
    parser.addArgument(blt::arg_builder("--tests").setHelp("Run the tests").setAction(blt::arg_action_t::STORE_TRUE).build());
    
    auto args = parser.parse_args(argc, argv);
    
    std::hash<blt::size_t> hash;
    BLT_TRACE0_STREAM << hash(500) << "\n";
    
    if (args.contains("--tests"))
    {
        //fb::test2();
        //fb::test3();
        //fb::test4();
        //fb::test5();
        
        fb::type_engine_t typeEngine;
        
        typeEngine.register_type("u8");
        typeEngine.register_type("bool");
        
        typeEngine.register_function("add", "u8", add_f, 2);
        typeEngine.register_function("sub", "u8", sub_f, 2);
        typeEngine.register_function("mul", "u8", mul_f, 2);
        typeEngine.register_function("div", "u8", div_f, 2);
        typeEngine.register_function("if", "u8", if_f, 3);
        typeEngine.register_function("equals_b", "bool", equals_b_f, 2);
        typeEngine.register_function("equals_n", "bool", equals_n_f, 2);
        typeEngine.register_function("less", "bool", less_f, 2);
        typeEngine.register_function("greater", "bool", greater_f, 2);
        typeEngine.register_function("not", "bool", not_f, 1);
        typeEngine.register_function("and_b", "bool", and_b_f, 2);
        typeEngine.register_function("and_n", "u8", and_n_f, 2);
        typeEngine.register_function("or_b", "bool", or_b_f, 2);
        typeEngine.register_function("or_n", "u8", or_n_f, 2);
        
        typeEngine.register_terminal_function("value", "u8", empty_f, value_init_f);
        typeEngine.register_terminal_function("bool_value", "bool", empty_f, bool_init_f);
        
        typeEngine.associate_input("add", {"u8", "u8"});
        typeEngine.associate_input("sub", {"u8", "u8"});
        typeEngine.associate_input("mul", {"u8", "u8"});
        typeEngine.associate_input("div", {"u8", "u8"});
        typeEngine.associate_input("if", {"bool", "u8", "u8"});
        typeEngine.associate_input("equals_b", {"bool", "bool"});
        typeEngine.associate_input("equals_n", {"u8", "u8"});
        typeEngine.associate_input("less", {"u8", "u8"});
        typeEngine.associate_input("greater", {"u8", "u8"});
        typeEngine.associate_input("not", {"bool"});
        typeEngine.associate_input("and_b", {"bool", "bool"});
        typeEngine.associate_input("or_b", {"bool", "bool"});
        typeEngine.associate_input("and_n", {"u8", "u8"});
        typeEngine.associate_input("or_n", {"u8", "u8"});
        
        //BLT_PRINT_PROFILE("Tree Construction");
        //BLT_PRINT_PROFILE("Tree Evaluation");
        //BLT_PRINT_PROFILE("Tree Destruction");
    }
    
    return 0;
}
