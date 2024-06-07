cmake_minimum_required(VERSION 3.21)

project(mem-alloc
        VERSION 0.1.0
        LANGUAGES C)


set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Package manager
include(cmake/CPM.cmake)

CPMAddPackage(
        NAME
        Unity 
        GIT_TAG
        v2.6.0
        GITHUB_REPOSITORY
        ThrowTheSwitch/Unity
)

# Library
add_library(mem-alloc STATIC src/pool_allocator.c)
target_include_directories(mem-alloc PUBLIC include)
target_compile_options(mem-alloc PRIVATE -Wall -Wfloat-conversion)

# Testing 
add_library(mem-alloc-lib-test STATIC src/pool_allocator.c)
target_include_directories(mem-alloc-lib-test PUBLIC include)
target_compile_options(mem-alloc-lib-test PRIVATE -Wall -Wfloat-conversion)
target_compile_definitions(mem-alloc-lib-test PUBLIC MEMP_DEBUG MEMP_STATS)

add_executable(mem-alloc-test test/test.c)
target_link_libraries(mem-alloc-test PUBLIC mem-alloc-lib-test unity)

# Benchmark
add_executable(mem-alloc-bench test/bench.c)
target_link_libraries(mem-alloc-bench PUBLIC mem-alloc)
