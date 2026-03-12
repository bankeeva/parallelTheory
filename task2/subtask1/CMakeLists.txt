cmake_minimum_required(VERSION 3.15)
project(task2)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

add_executable(matvec_omp
    task2_1.cpp)

target_link_libraries(matvec_omp PRIVATE OpenMP::OpenMP_CXX)

target_compile_options(matvec_omp PRIVATE -O2)