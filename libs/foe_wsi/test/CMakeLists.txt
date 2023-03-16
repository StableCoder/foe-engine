# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-WSI-Test COMMAND test_foe_wsi)
add_executable(test_foe_wsi keyboard.cpp mouse.cpp)

# Definition
target_link_libraries(test_foe_wsi PRIVATE Catch2::Catch2WithMain foe_wsi)

target_code_coverage(
  test_foe_wsi AUTO ALL EXCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
