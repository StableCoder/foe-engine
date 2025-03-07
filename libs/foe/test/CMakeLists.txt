# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-Core-Test COMMAND test_foe_core)
add_executable(test_foe_core)
add_dependencies(test_foe_core test_foe_core_plugin_so)

add_test(NAME FoE-Test-Split-Thread-Pool
         COMMAND test_foe_core_split_thread_pool)
add_executable(test_foe_core_split_thread_pool)

# Main Definition
target_sources(
  test_foe_core
  PRIVATE algorithm.cpp
          c_header_compatibility.c
          delimited_string.cpp
          filesystem.cpp
          hex.cpp
          logger.cpp
          managed_memory.cpp
          memory_alignment.cpp
          memory_mapped_file.cpp
          plugin.cpp
          result.cpp
          search_paths.cpp)

target_link_libraries(test_foe_core PRIVATE Catch2::Catch2WithMain foe_core)

target_compile_definitions(
  test_foe_core PRIVATE TEST_PLUGIN_DIR="$<TARGET_FILE:test_foe_core_plugin_so>"
                        TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_code_coverage(
  test_foe_core
  AUTO
  ALL
  OBJECTS
  foe_core
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)

# Split Thread Pool Definition
target_sources(test_foe_core_split_thread_pool PRIVATE split_thread_pool.cpp)

target_link_libraries(test_foe_core_split_thread_pool
                      PRIVATE Catch2::Catch2WithMain foe_core)

target_code_coverage(
  test_foe_core_split_thread_pool
  AUTO
  ALL
  OBJECTS
  foe_core
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)

# Directories
add_subdirectory(chrono)
add_subdirectory(test_plugin_so)
