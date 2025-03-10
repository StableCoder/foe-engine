# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_vk_imgui)

# Declaration
add_library(foe_graphics_vk_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_graphics_vk_imgui PUBLIC foe_graphics_imgui
                                                   foe_graphics_vk foe_imgui)

target_code_coverage(foe_graphics_vk_imgui)

if(MSVC)
  target_compile_options(foe_graphics_vk_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_vk_imgui PRIVATE -fno-exceptions)
endif()

# install
install(
  TARGETS foe_graphics_vk_imgui
  EXPORT foe_graphics_vk_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics_vk_imgui
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics_vk_imgui-config.cmake DESTINATION share/cmake/foe)
