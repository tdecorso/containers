cmake_minimum_required(VERSION 3.31)

project(containers LANGUAGES C VERSION 0.1)

set(CMAKE_BUILD_TYPE Debug)

add_library(containers
             src/array.c
              src/list.c
             src/queue.c
             src/stack.c
           src/hashmap.c
    src/hash_functions.c
              src/tree.c)

target_include_directories(containers
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

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

# ── Install ────────────────────────────────────────────────────────

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS containers
    EXPORT containersTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES include/containers.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT containersTargets
    FILE containersTargets.cmake
    NAMESPACE containers::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/containers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/containersConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/containersConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/containersConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/containers)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/containersConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/containersConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/containers)
