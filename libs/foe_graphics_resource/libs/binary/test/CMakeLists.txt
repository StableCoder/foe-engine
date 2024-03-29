# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_graphics_resource_binary)
add_test(NAME FoE-Graphics-Resource-Binary-Test
         COMMAND test_foe_graphics_resource_binary)

# Definition
target_sources(test_foe_graphics_resource_binary
               PRIVATE c_header_compatibility.c result.cpp)

target_link_libraries(
  test_foe_graphics_resource_binary PRIVATE Catch2::Catch2WithMain
                                            foe_graphics_resource_binary)

target_code_coverage(
  test_foe_graphics_resource_binary
  AUTO
  ALL
  OBJECTS
  foe_graphics_resource_binary
  EXCLUDE
  .*/test/.*)
