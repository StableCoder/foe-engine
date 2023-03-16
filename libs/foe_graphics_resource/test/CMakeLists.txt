# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_graphics_resource)
add_test(NAME FoE-Graphics-Resource-Test COMMAND test_foe_physics)

# Definition
target_sources(
  test_foe_graphics_resource PRIVATE binary_foeImageCreateInfo.cpp
                                     c_header_compatibility.c result.cpp)

target_link_libraries(test_foe_graphics_resource PRIVATE Catch2::Catch2WithMain
                                                         foe_graphics_resource)

target_code_coverage(
  test_foe_graphics_resource
  AUTO
  ALL
  OBJECTS
  foe_graphics_resource
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
