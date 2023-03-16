# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-ImGui-VK-Test COMMAND test_foe_imgui_vk)
add_executable(test_foe_imgui_vk)

# Definition
target_sources(test_foe_imgui_vk PRIVATE c_header_compatibility.c result.cpp)

target_link_libraries(test_foe_imgui_vk PRIVATE Catch2::Catch2WithMain
                                                foe_imgui_vk)

target_code_coverage(
  test_foe_imgui_vk
  AUTO
  ALL
  OBJECTS
  foe_imgui_vk
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
