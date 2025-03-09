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
  foe_graphics_vk PUBLIC foe_graphics vulkan_memory_allocator vk_struct_cleanup
                         vk_struct_compare vk_value_serialization)
target_link_libraries(foe_graphics_vk PRIVATE khronos_mini_libs)

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

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
