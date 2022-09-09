# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Graphics-Test COMMAND test_foe_graphics)
add_executable(test_foe_graphics)

# Definition
target_sources(test_foe_graphics PRIVATE c_header_compatibility.c stub.cpp)

target_link_libraries(test_foe_graphics PRIVATE catch foe_graphics)

target_code_coverage(
  test_foe_graphics
  AUTO
  ALL
  OBJECTS
  foe_graphics
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
