# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_imex)
add_test(NAME FoE-ImEx-Test COMMAND test_foe_imex)

# Definition
target_sources(test_foe_imex PRIVATE c_header_compatibility.c importer.cpp
                                     result.cpp)

target_link_libraries(test_foe_imex PRIVATE Catch2::Catch2WithMain foe_imex)

target_code_coverage(
  test_foe_imex
  AUTO
  ALL
  OBJECTS
  foe_imex
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
