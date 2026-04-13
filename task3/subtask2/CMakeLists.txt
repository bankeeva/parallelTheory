cmake_minimum_required(VERSION 3.15)
project(task32)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

add_executable(srevcl 
            main.cpp)
target_link_libraries(srevcl PRIVATE Threads::Threads)
target_compile_options(srevcl PRIVATE -O2)