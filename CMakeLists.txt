cmake_minimum_required(VERSION 3.31)

project(containers)

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

add_library(containers 
             src/array.c 
              src/list.c 
             src/queue.c 
             src/stack.c 
           src/hashmap.c 
    src/hash_functions.c
              src/tree.c)

add_executable(test_array    tests/test_array.c)
add_executable(test_list     tests/test_list.c)
add_executable(test_queue    tests/test_queue.c)
add_executable(test_stack    tests/test_stack.c)
add_executable(test_hashmap  tests/test_hashmap.c)
add_executable(test_tree     tests/test_tree.c)

target_link_libraries(test_array    PRIVATE containers)
target_link_libraries(test_list     PRIVATE containers)
target_link_libraries(test_queue    PRIVATE containers)
target_link_libraries(test_stack    PRIVATE containers)
target_link_libraries(test_hashmap  PRIVATE containers)
target_link_libraries(test_tree     PRIVATE containers)
