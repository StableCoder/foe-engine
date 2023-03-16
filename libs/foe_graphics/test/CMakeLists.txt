# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-Graphics-Test COMMAND test_foe_graphics)
add_executable(test_foe_graphics)

# Definition
target_sources(test_foe_graphics PRIVATE c_header_compatibility.c result.cpp
                                         stub.cpp)

target_link_libraries(test_foe_graphics PRIVATE Catch2::Catch2WithMain
                                                foe_graphics)

target_code_coverage(
  test_foe_graphics
  AUTO
  ALL
  OBJECTS
  foe_graphics
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
