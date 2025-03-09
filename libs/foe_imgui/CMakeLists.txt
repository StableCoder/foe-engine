# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imgui)

# Declaration
add_library(foe_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_imgui PUBLIC foe_wsi imgui imgui_file_dialog)

target_code_coverage(foe_imgui)

if(MSVC)
  target_compile_options(foe_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_imgui PRIVATE -fno-exceptions)
endif()

# Auxiliary Target
add_subdirectory(libs/vulkan)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
