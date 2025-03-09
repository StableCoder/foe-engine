# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
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
set_property(TARGET foe_xr_openxr PROPERTY EXPORT_NAME xr_openxr)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(
  foe_xr_openxr
  PUBLIC foe_xr OpenXR::openxr_loader
  PRIVATE $<BUILD_INTERFACE:khronos_mini_libs>)

target_code_coverage(foe_xr_openxr)

if(DEFINED GLM_INCLUDE_DIRS)
  target_include_directories(foe_xr_openxr PUBLIC ${GLM_INCLUDE_DIRS}
                                                  ${GLM_INCLUDE_DIRS}/include)
elseif(TARGET glm::glm)
  target_link_libraries(foe_xr_openxr PUBLIC glm::glm)
else()
  target_link_libraries(foe_xr_openxr PUBLIC glm::glm-header-only)
endif()

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
