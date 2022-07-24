# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_imex)
add_test(NAME FoE-ImEx-Test COMMAND test_foe_imex)

# Definition
target_sources(test_foe_imex PRIVATE c_header_compatibility.c error_code.cpp
                                     importer.cpp)

target_link_libraries(test_foe_imex PRIVATE catch foe_imex)

target_code_coverage(
  test_foe_imex
  AUTO
  ALL
  OBJECTS
  foe_imex
  EXCLUDE
  .*/test/.*)
