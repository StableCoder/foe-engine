# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_imex_binary)
add_test(NAME FoE-ImEx-Binary-Test COMMAND test_foe_imex_binary)

# Definition
target_sources(test_foe_imex_binary PRIVATE c_header_compatibility.c result.cpp)

target_compile_definitions(
  test_foe_imex_binary PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(test_foe_imex_binary PRIVATE Catch2::Catch2WithMain
                                                   foe_imex_binary)

target_code_coverage(
  test_foe_imex_binary
  AUTO
  ALL
  OBJECTS
  foe_imex_binary
  EXCLUDE
  .*/test/.*)
