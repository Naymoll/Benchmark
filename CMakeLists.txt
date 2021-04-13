cmake_minimum_required(VERSION 3.17)
project(bench)

set(CMAKE_CXX_STANDARD 17)

add_executable(bench main.cpp)

add_subdirectory(benchmark)
target_link_libraries(bench benchmark::benchmark)
