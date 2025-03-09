# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_graphics SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_graphics PUBLIC Vulkan::Vulkan foe_core)

target_code_coverage(foe_graphics)

if(MSVC)
  target_compile_options(foe_graphics PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
