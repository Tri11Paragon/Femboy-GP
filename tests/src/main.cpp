#include <iostream>
#include <utility>
#include <memory>
#include <blt/std/logging.h>

struct data {
    float f;
    int i;
    char c;
};

int main()
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
    std::cout << static_cast<void*>(new_ptr) << " : " << remaining_bytes << " | " << (buffer - new_ptr + sizeof(char)) << std::endl;
    offset += (buffer - new_ptr + sizeof(char));
    
    remaining_bytes = size - offset;
    void_ptr = reinterpret_cast<void*>(&buffer[offset]);
    new_ptr = static_cast<char*>(std::align(alignof(data), sizeof(data), void_ptr, remaining_bytes));
    std::cout << static_cast<void*>(new_ptr) << " : " << remaining_bytes << " | " << (buffer - new_ptr + sizeof(data)) << std::endl;
    offset += (buffer - new_ptr + sizeof(data));
    
    
    delete[](buffer);
    
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
