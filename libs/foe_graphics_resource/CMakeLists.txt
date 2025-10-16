# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_resource)

# Dependencies
find_package(ImageMagick REQUIRED MagickCore)

# Declaration
add_library(foe_graphics_resource SHARED)
add_library(foe::graphics::resource ALIAS foe_graphics_resource)

# Definition
generate_export_header(
  foe_graphics_resource EXPORT_MACRO_NAME FOE_GFX_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/export.h)

target_link_libraries(
  foe_graphics_resource
  PUBLIC foe_core
         foe_ecs
         foe_graphics
         foe_graphics_vk
         foe_model
         foe_model_assimp
         foe_simulation
  PRIVATE vk_struct_cleanup vk_struct_compare ImageMagick::MagickCore)

target_code_coverage(foe_graphics_resource)

if(MSVC)
  target_compile_options(foe_graphics_resource PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_resource PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_graphics_resource
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/export.h
         include/foe/graphics/resource/binary.h
         include/foe/graphics/resource/cleanup.h
         include/foe/graphics/resource/compare.h
         include/foe/graphics/resource/image_create_info.h
         include/foe/graphics/resource/image.hpp
         include/foe/graphics/resource/material_create_info.h
         include/foe/graphics/resource/material.hpp
         include/foe/graphics/resource/mesh_create_info.h
         include/foe/graphics/resource/mesh.hpp
         include/foe/graphics/resource/registration.h
         include/foe/graphics/resource/result.h
         include/foe/graphics/resource/shader_create_info.h
         include/foe/graphics/resource/shader.hpp
         include/foe/graphics/resource/type_defs.h
         include/foe/graphics/resource/vertex_descriptor_create_info.h
         include/foe/graphics/resource/vertex_descriptor.hpp)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_resource PROPERTY EXPORT_NAME
                                                   foe::graphics::resource)

install(
  TARGETS foe_graphics_resource
  EXPORT foe_graphics_resource
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics_resource DESTINATION share/cmake/foe)

install(FILES foe_graphics_resource-config.cmake DESTINATION share/cmake/foe)
