# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_xr_openxr_vk)

# Declaration
add_library(foe_xr_openxr_vk)
add_library(foe::xr::openxr::vk ALIAS foe_xr_openxr_vk)

# Definition
set_target_properties(foe_xr_openxr_vk PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(
  foe_xr_openxr_vk EXPORT_MACRO_NAME FOE_OPENXR_VK_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/openxr/vk/export.h)

target_link_libraries(
  foe_xr_openxr_vk
  PUBLIC foe_xr_openxr foe_graphics_vk
  PRIVATE vk_result_serialization)

target_code_coverage(foe_xr_openxr_vk)

target_sources(
  foe_xr_openxr_vk
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/openxr/vk/export.h
         include/foe/xr/openxr/vk/render_graph_jobs_swapchain.hpp
         include/foe/xr/openxr/vk/result.h
         include/foe/xr/openxr/vk/vulkan.h)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_xr_openxr_vk PROPERTY EXPORT_NAME foe::xr::openxr::vk)

install(
  TARGETS foe_xr_openxr_vk
  EXPORT foe_xr_openxr_vk
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_xr_openxr_vk DESTINATION share/cmake/foe)

install(FILES foe_xr_openxr_vk-config.cmake DESTINATION share/cmake/foe)
