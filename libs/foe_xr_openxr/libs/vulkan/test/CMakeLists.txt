# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-OpenXR-VK-Test COMMAND test_foe_xr_openxr_vk)
add_executable(test_foe_xr_openxr_vk)

# Definition
target_sources(test_foe_xr_openxr_vk PRIVATE result.cpp)

target_link_libraries(test_foe_xr_openxr_vk PRIVATE Catch2::Catch2WithMain
                                                    foe_xr_openxr_vk)

target_code_coverage(
  test_foe_xr_openxr_vk
  AUTO
  ALL
  OBJECTS
  foe_xr_openxr_vk
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
