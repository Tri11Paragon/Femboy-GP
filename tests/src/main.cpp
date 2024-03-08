#include <iostream>
#include <utility>
#include <memory>
#include <blt/std/logging.h>
#include <blt/parse/argparse.h>
#include <lilfbtf/test2.h>
#include <lilfbtf/test3.h>
#include "blt/profiling/profiler_v2.h"

struct data {
    float f;
    int i;
    char c;
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
    
    if (args.contains("--tests"))
    {
        //fb::test2();
        fb::test3();
        
        BLT_PRINT_PROFILE("Tree Construction");
        BLT_PRINT_PROFILE("Tree Evaluation");
        BLT_PRINT_PROFILE("Tree Destruction");
    }
    
    return 0;
}
