# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_vk)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_graphics_vk SHARED)
add_library(foe::graphics::vk ALIAS foe_graphics_vk)

# Definition
generate_export_header(
  foe_graphics_vk EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h)

target_link_libraries(
  foe_graphics_vk
  PUBLIC foe_graphics Vulkan::Vulkan vulkan_memory_allocator
  PRIVATE vk_struct_cleanup vk_struct_compare vk_value_serialization
          vk_result_serialization)

target_code_coverage(foe_graphics_vk)

if(APPLE)
  target_compile_definitions(foe_graphics_vk PUBLIC VK_USE_PLATFORM_MACOS_MVK)
endif()

if(MSVC)
  target_compile_options(foe_graphics_vk PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_vk PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_graphics_vk
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h
         include/foe/graphics/vk/render_graph/job/blit_image.hpp
         include/foe/graphics/vk/render_graph/job/copy_image.hpp
         include/foe/graphics/vk/render_graph/job/export_image.hpp
         include/foe/graphics/vk/render_graph/job/import_image.hpp
         include/foe/graphics/vk/render_graph/job/present_image.hpp
         include/foe/graphics/vk/render_graph/job/resolve_image.hpp
         include/foe/graphics/vk/render_graph/job/synchronize.hpp
         include/foe/graphics/vk/render_graph/resource/image.hpp
         include/foe/graphics/vk/binary.h
         include/foe/graphics/vk/cleanup.h
         include/foe/graphics/vk/compare.h
         include/foe/graphics/vk/debug_callback.h
         include/foe/graphics/vk/error_images.h
         include/foe/graphics/vk/format.h
         include/foe/graphics/vk/fragment_descriptor.h
         include/foe/graphics/vk/fragment_descriptor_pool.h
         include/foe/graphics/vk/image.h
         include/foe/graphics/vk/mesh.h
         include/foe/graphics/vk/model.h
         include/foe/graphics/vk/pipeline_pool.h
         include/foe/graphics/vk/queue_family.h
         include/foe/graphics/vk/render_graph.hpp
         include/foe/graphics/vk/render_pass_pool.h
         include/foe/graphics/vk/render_target.h
         include/foe/graphics/vk/render_view_pool.h
         include/foe/graphics/vk/result.h
         include/foe/graphics/vk/runtime.h
         include/foe/graphics/vk/sample_count.h
         include/foe/graphics/vk/session.h
         include/foe/graphics/vk/shader.h
         include/foe/graphics/vk/swapchain.h
         include/foe/graphics/vk/vertex_descriptor.h
         include/foe/graphics/vk/vk_binary.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_vk PROPERTY EXPORT_NAME foe::graphics::vk)

install(
  TARGETS foe_graphics_vk vulkan_memory_allocator vk_struct_cleanup
          vk_struct_compare vk_value_serialization vk_result_serialization
  EXPORT foe_graphics_vk
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics_vk DESTINATION share/cmake/foe)

install(FILES foe_graphics_vk-config.cmake DESTINATION share/cmake/foe)
