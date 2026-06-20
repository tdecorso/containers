cmake_minimum_required(VERSION 3.31)

project(containers)

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

add_library(containers src/array.c)

add_executable(test_array tests/array.c)
target_link_libraries(test_array PRIVATE containers)
