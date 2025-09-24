# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imgui_vk)

# Declaration
add_library(foe_imgui_vk)
add_library(foe::imgui::vk ALIAS foe_imgui_vk)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(
  foe_imgui_vk
  PUBLIC foe_graphics_vk foe_imgui
  PRIVATE vk_result_serialization)

target_code_coverage(foe_imgui_vk)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_imgui_vk PROPERTY EXPORT_NAME imgui::vk)

install(
  TARGETS foe_imgui_vk
  EXPORT foe_imgui_vk
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_imgui_vk
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_imgui_vk-config.cmake DESTINATION share/cmake/foe)
