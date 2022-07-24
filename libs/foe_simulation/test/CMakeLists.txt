# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_simulation)
add_test(NAME FoE-Simulation-Test COMMAND test_foe_simulation)

# Definition
target_sources(
  test_foe_simulation PRIVATE c_header_compatibility.c error_code.cpp
                              group_data.cpp simulation.cpp)

target_link_libraries(test_foe_simulation PRIVATE catch foe_simulation)

target_code_coverage(
  test_foe_simulation
  AUTO
  ALL
  OBJECTS
  foe_simulation
  EXCLUDE
  .*/test/.*)
