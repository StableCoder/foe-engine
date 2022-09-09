# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-OpenXR-VK-Test COMMAND test_foe_xr_openxr_vk)
add_executable(test_foe_xr_openxr_vk)

# Definition
target_sources(test_foe_xr_openxr_vk PRIVATE result.cpp)

target_link_libraries(test_foe_xr_openxr_vk PRIVATE catch foe_xr_openxr_vk)

target_code_coverage(
  test_foe_xr_openxr_vk
  AUTO
  ALL
  OBJECTS
  foe_xr_openxr_vk
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
