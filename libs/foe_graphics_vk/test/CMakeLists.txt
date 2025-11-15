# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-Graphics-VK-Test COMMAND test_foe_graphics_vk)
add_executable(test_foe_graphics_vk)

# Definition
target_sources(
  test_foe_graphics_vk
  PRIVATE c_header_compatibility.c
          custom_main.cpp
          format.cpp
          render_graph_image_resource.cpp
          render_graph.cpp
          result.cpp
          sample_count.cpp
          vk_struct_VkPipelineDepthStencilStateCreateInfo.cpp
          vk_struct_VkStencilOpState.cpp)

if(BUILD_VULKAN_RUNTIME_TESTS)
  target_sources(test_foe_graphics_vk PRIVATE debug_callback.cpp runtime.cpp
                                              session.cpp shader.cpp upload.cpp)
endif()

target_link_libraries(
  test_foe_graphics_vk
  PRIVATE Catch2::Catch2 foe_graphics_vk
  PRIVATE vk_struct_compare)

target_code_coverage(
  test_foe_graphics_vk
  AUTO
  ALL
  OBJECTS
  foe_graphics_vk
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
