# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_xr_openxr_vk)

# Declaration
add_library(foe_xr_openxr_vk)
add_library(foe::xr::openxr::vk ALIAS foe_xr_openxr_vk)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(
  foe_xr_openxr_vk
  PUBLIC foe_xr_openxr foe_graphics_vk
  PRIVATE vk_result_serialization)

target_code_coverage(foe_xr_openxr_vk)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_xr_openxr_vk PROPERTY EXPORT_NAME xr::openxr::vk)

install(
  TARGETS foe_xr_openxr_vk
  EXPORT foe_xr_openxr_vk
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_xr_openxr_vk
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_xr_openxr_vk-config.cmake DESTINATION share/cmake/foe)
