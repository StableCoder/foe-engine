# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_wsi_glfw3)

# To use this library as the default WSI library, use set WSI_LIBRARY to 'glfw3'
# in the CMake project settings

# Dependencies
find_package(glfw3 REQUIRED)

# Declaration
if(WSI_LOADER)
  # If not directly using this library, eg. through a loader, make sure it is
  # built as a shared object
  add_library(foe_wsi_glfw3 SHARED)
else()
  add_library(foe_wsi_glfw3)
endif()

# Definition
add_subdirectory(src)

target_link_libraries(
  foe_wsi_glfw3
  PUBLIC foe_wsi glfw
  PRIVATE khronos_mini_libs)

target_code_coverage(foe_wsi_glfw3)

if(MSVC)
  target_compile_options(foe_wsi_glfw3 PRIVATE /EHsc)
else()
  target_compile_options(foe_wsi_glfw3 PRIVATE -fno-exceptions)
endif()

if(NOT WSI_LOADER AND WSI_LIBRARY STREQUAL "glfw3")
  # Export the WSI_EXPORT header from this library
  generate_export_header(
    foe_wsi_glfw3 EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)
else()
  # Don't export the WSI_EXPORT header from this library, use local use only
  generate_export_header(
    foe_wsi_glfw3 EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)
endif()
target_sources(
  foe_wsi_glfw3 PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
                       FILES ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
