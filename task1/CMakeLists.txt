cmake_minimum_required(VERSION 3.15)
project(Task1)

set(CMAKE_CXX_STANDARD 17)

option(USE_DOUBLE "Use double instead of float" OFF)

add_executable(Task1
        main.cpp)

if(USE_DOUBLE)
        target_compile_definitions(Task1 PRIVATE USE_DOUBLE=1)
else()
        target_compile_definitions(Task1 PRIVATE USE_DOUBLE=0)
endif()
