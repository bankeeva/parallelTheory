cmake_minimum_required(VERSION 3.15)
project(task31)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(matvec
    main.cpp)

target_link_libraries(matvec PRIVATE Threads::Threads)
target_compile_options(matvec PRIVATE -O2)