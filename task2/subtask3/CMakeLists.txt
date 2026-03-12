cmake_minimum_required(VERSION 3.15)
project(slau)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

add_executable(slau_v1 var1.cpp)
target_link_libraries(slau_v1 PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(slau_v1 PRIVATE -O2)

add_executable(slau_v2 var2.cpp)
target_link_libraries(slau_v2 PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(slau_v2 PRIVATE -O2)

add_executable(ansche analysisSchedule.cpp)
target_link_libraries(ansche PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(ansche PRIVATE -O2)
