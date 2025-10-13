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
add_library(foe::xr::openxr ALIAS foe_xr_openxr)

# Definition
generate_export_header(
  foe_xr_openxr EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/export.h)

target_link_libraries(
  foe_xr_openxr
  PUBLIC foe_xr OpenXR::openxr_loader
  PRIVATE xr_result_serialization)

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

target_sources(
  foe_xr_openxr
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/export.h
         include/foe/xr/openxr/camera_math.hpp
         include/foe/xr/openxr/result.h
         include/foe/xr/openxr/runtime.h
         include/foe/xr/openxr/session.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/vulkan)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_xr_openxr PROPERTY EXPORT_NAME xr::openxr)

install(
  TARGETS foe_xr_openxr xr_result_serialization
  EXPORT foe_xr_openxr
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_xr_openxr
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_xr_openxr-config.cmake DESTINATION share/cmake/foe)
