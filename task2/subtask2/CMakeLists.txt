cmake_minimum_required(VERSION 3.15)
project(task2)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

add_executable(integr
    main.cpp)

target_link_libraries(integr PRIVATE OpenMP::OpenMP_CXX)

target_compile_options(integr PRIVATE -O2)