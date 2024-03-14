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

const fb::func_t_call_t add_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<double>() + args[1]->value().any_cast<double>());
};
const fb::func_t_call_t sub_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<double>() - args[1]->value().any_cast<double>());
};
const fb::func_t_call_t mul_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<double>() * args[1]->value().any_cast<double>());
};
const fb::func_t_call_t div_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    auto dim = args[1]->value().any_cast<double>();
    if (dim == 0)
        us.setValue(0);
    else
        us.setValue(args[0]->value().any_cast<double>() + dim);
};

const fb::func_t_call_t value_f = [](fb::func_t&, blt::span<fb::detail::node_t*>) {};
const fb::func_t_call_t if_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    if (args[0]->value().any_cast<bool>())
        us.setValue(args[1]->value().any_cast<double>());
    else
        us.setValue(args[2]->value().any_cast<double>());
};
const fb::func_t_call_t equals_b_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<bool>() == args[1]->value().any_cast<bool>());
};
const fb::func_t_call_t equals_n_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<double>() == args[1]->value().any_cast<double>());
};
const fb::func_t_call_t less_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<double>() < args[1]->value().any_cast<double>());
};
const fb::func_t_call_t greater_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<double>() > args[1]->value().any_cast<double>());
};
const fb::func_t_call_t not_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) { us.setValue(!args[0]->value().any_cast<bool>()); };
const fb::func_t_call_t and_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<bool>() && args[1]->value().any_cast<bool>());
};
const fb::func_t_call_t or_f = [](fb::func_t& us, blt::span<fb::detail::node_t*> args) {
    us.setValue(args[0]->value().any_cast<bool>() || args[1]->value().any_cast<bool>());
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
        
        
        BLT_PRINT_PROFILE("Tree Construction");
        BLT_PRINT_PROFILE("Tree Evaluation");
        BLT_PRINT_PROFILE("Tree Destruction");
    }
    
    return 0;
}
