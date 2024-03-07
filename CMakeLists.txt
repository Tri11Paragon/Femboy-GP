cmake_minimum_required(VERSION 3.25)
project(lilfbtf5 VERSION 0.1.0)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)

if(!MSVC)
add_compile_options(-march=native)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libs/BLT)

file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE PROJECT_TESTS_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/tests/src/*.cpp")

add_library(lilfbtf5 ${PROJECT_BUILD_FILES})

target_include_directories(lilfbtf5 PUBLIC include/)
target_link_libraries(lilfbtf5 PUBLIC BLT)

message("Is MSVC ${MSVC}")

if(MSVC)
    message("Using MSVC")
    #target_compile_options(${BLT_TARGET} PRIVATE /W4)
else()
    message("Using non MSVC options")
    # perhaps we should warn on unused variables, but BLT will have lots of them.
    target_compile_options(lilfbtf5 PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
    target_link_options(lilfbtf5 PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
endif()

if (${ENABLE_ADDRSAN} MATCHES ON)
    target_compile_options(lilfbtf5 PRIVATE -fsanitize=address)
    target_link_options(lilfbtf5 PRIVATE -fsanitize=address)
endif ()

if (${ENABLE_UBSAN} MATCHES ON)
    target_compile_options(lilfbtf5 PRIVATE -fsanitize=undefined)
    target_link_options(lilfbtf5 PRIVATE -fsanitize=undefined)
endif ()

if (${ENABLE_TSAN} MATCHES ON)
    target_compile_options(lilfbtf5 PRIVATE -fsanitize=thread)
    target_link_options(lilfbtf5 PRIVATE -fsanitize=thread)
endif ()

project(lilfbtf5_test)

add_executable(lilfbtf5_test ${PROJECT_TESTS_BUILD_FILES})

target_include_directories(lilfbtf5_test PUBLIC tests/include/)
target_link_libraries(lilfbtf5_test PUBLIC lilfbtf5)


if(MSVC)
    message("Using MSVC")
    #target_compile_options(${BLT_TARGET} PRIVATE /W4)
else()
    message("Using non MSVC options")
    # perhaps we should warn on unused variables, but BLT will have lots of them.
    target_compile_options(lilfbtf5_test PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
    target_link_options(lilfbtf5_test PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
endif()


if (${ENABLE_ADDRSAN} MATCHES ON)
    target_compile_options(lilfbtf5_test PRIVATE -fsanitize=address)
    target_link_options(lilfbtf5_test PRIVATE -fsanitize=address)
endif ()

if (${ENABLE_UBSAN} MATCHES ON)
    target_compile_options(lilfbtf5_test PRIVATE -fsanitize=undefined)
    target_link_options(lilfbtf5_test PRIVATE -fsanitize=undefined)
endif ()

if (${ENABLE_TSAN} MATCHES ON)
    target_compile_options(lilfbtf5_test PRIVATE -fsanitize=thread)
    target_link_options(lilfbtf5_test PRIVATE -fsanitize=thread)
endif ()

project(lilfbtf5)