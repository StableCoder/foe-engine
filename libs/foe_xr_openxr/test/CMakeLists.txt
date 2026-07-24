# Copyright (C) 2022-2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-XR-OpenXR-Test COMMAND test_foe_xr_openxr)
add_executable(test_foe_xr_openxr)

set_target_properties(test_foe_xr_openxr PROPERTIES FOLDER "Tests")

# Definition
target_sources(test_foe_xr_openxr PRIVATE result.cpp)

target_link_libraries(test_foe_xr_openxr PRIVATE Catch2::Catch2WithMain
                                                 foe_xr_openxr)

target_code_coverage(
  test_foe_xr_openxr
  AUTO
  ALL
  OBJECTS
  foe_xr_openxr
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
