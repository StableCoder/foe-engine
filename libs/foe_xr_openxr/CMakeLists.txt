# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_xr_openxr)

# To use this library as the default XR library, use set XR_LIBRARY to 'openxr'
# in the CMake project settings

# Dependencies
find_package(glm REQUIRED)
find_package(OpenXR REQUIRED)

# Declaration
if(XR_LOADER)
  # If not directly using this library, eg. through a loader, make sure it is
  # built as a shared object
  add_library(foe_xr_openxr SHARED)
else()
  add_library(foe_xr_openxr)
endif()

# Definition
add_subdirectory(src)

generate_export_header(
  foe_xr_openxr EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/xr/export.h)

target_include_directories(
  foe_xr_openxr PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                       ${GLM_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS}/include)

target_link_libraries(foe_xr_openxr PUBLIC foe_xr OpenXR::openxr_loader)
target_link_libraries(foe_xr_openxr PRIVATE khronos_mini_libs)

target_code_coverage(foe_xr_openxr)

if(MSVC)
  target_compile_options(foe_xr_openxr PRIVATE /EHsc)
else()
  target_compile_options(foe_xr_openxr PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/vulkan)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
