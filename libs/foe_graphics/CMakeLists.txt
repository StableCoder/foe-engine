# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_graphics)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_graphics SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_graphics EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/graphics/export.h)

target_include_directories(foe_graphics
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_graphics PUBLIC Vulkan::Vulkan foe)

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
