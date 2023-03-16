# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Test Setup
enable_testing()
find_package(Catch2 3 REQUIRED)

add_executable(test_foe_imgui state.cpp)
target_link_libraries(test_foe_imgui PRIVATE foe_imgui Catch2::Catch2WithMain)
target_code_coverage(
  test_foe_imgui
  AUTO
  ALL
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  OBJECTS
  foe_imgui)

add_test(NAME FoE-ImGui-Tests COMMAND test_foe_imgui)
