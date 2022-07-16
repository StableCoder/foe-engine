# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Test COMMAND test_foe)
add_executable(test_foe)

add_test(NAME FoE-Test-Thread-Pool COMMAND test_foe_thread_pool)
add_executable(test_foe_thread_pool)

add_test(NAME FoE-Test-Split-Thread-Pool COMMAND test_foe_split_thread_pool)
add_executable(test_foe_split_thread_pool)

# Main Definition
target_sources(
  test_foe
  PRIVATE algorithm.cpp
          c_header_compatibility.c
          command_string_runner.cpp
          data_pool_custom.cpp
          data_pool_int.cpp
          data_pool_multi.cpp
          delimited_string.cpp
          filesystem.cpp
          plugin.cpp
          search_paths.cpp)

target_link_libraries(test_foe PRIVATE catch foe)

target_compile_definitions(
  test_foe PRIVATE TEST_PLUGIN_DIR="$<TARGET_FILE:test_foe_plugin_so>"
                   TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_code_coverage(
  test_foe
  AUTO
  ALL
  OBJECTS
  foe
  EXCLUDE
  .*/test/.*)

# Thread Pool Definition
target_sources(test_foe_thread_pool PRIVATE thread_pool_error_code.cpp
                                            thread_pool.cpp)

target_link_libraries(test_foe_thread_pool PRIVATE catch foe)

target_code_coverage(
  test_foe_thread_pool
  AUTO
  ALL
  OBJECTS
  foe
  EXCLUDE
  .*/test/.*)

# Split Thread Pool Definition
target_sources(test_foe_split_thread_pool PRIVATE split_thread_pool.cpp)

target_link_libraries(test_foe_split_thread_pool PRIVATE catch foe)

target_code_coverage(
  test_foe_split_thread_pool
  AUTO
  ALL
  OBJECTS
  foe
  EXCLUDE
  .*/test/.*)

# Directories
add_subdirectory(chrono)
add_subdirectory(log)
add_subdirectory(storage)
add_subdirectory(test_plugin_so)
