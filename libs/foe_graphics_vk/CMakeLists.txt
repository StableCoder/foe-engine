# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_vk)

# Declaration
add_library(foe_graphics_vk SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(
  foe_graphics_vk
  PUBLIC foe_graphics
         $<BUILD_INTERFACE:vulkan_memory_allocator>
         $<BUILD_INTERFACE:vk_struct_cleanup>
         $<BUILD_INTERFACE:vk_struct_compare>
         $<BUILD_INTERFACE:vk_value_serialization>
  PRIVATE $<BUILD_INTERFACE:khronos_mini_libs>)

target_code_coverage(foe_graphics_vk)

if(APPLE)
  target_compile_definitions(foe_graphics_vk PUBLIC VK_USE_PLATFORM_MACOS_MVK)
endif()

if(MSVC)
  target_compile_options(foe_graphics_vk PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_vk PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_vk PROPERTY EXPORT_NAME graphics_vk)

install(
  TARGETS foe_graphics_vk
  EXPORT foe_graphics_vk
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics_vk
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics_vk-config.cmake DESTINATION share/cmake/foe)
