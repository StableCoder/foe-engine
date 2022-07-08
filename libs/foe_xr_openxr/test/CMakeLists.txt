# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-OpenXR-Test COMMAND test_foe_openxr)
add_executable(test_foe_openxr)

# Definition
target_sources(test_foe_openxr PRIVATE c_header_compatability.c error_code.cpp)

target_link_libraries(test_foe_openxr PRIVATE catch foe_xr_openxr)

target_code_coverage(
  test_foe_openxr
  AUTO
  ALL
  OBJECTS
  foe_xr_openxr
  EXCLUDE
  .*/test/.*)
