# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_resource EXPORT_MACRO_NAME FOE_GFX_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/export.h)

target_sources(
  foe_graphics_resource
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/resource/export.h
         foe/graphics/resource/binary.h
         foe/graphics/resource/cleanup.h
         foe/graphics/resource/compare.h
         foe/graphics/resource/image_create_info.h
         foe/graphics/resource/image.hpp
         foe/graphics/resource/material_create_info.h
         foe/graphics/resource/material.hpp
         foe/graphics/resource/mesh_create_info.h
         foe/graphics/resource/mesh.hpp
         foe/graphics/resource/registration.h
         foe/graphics/resource/result.h
         foe/graphics/resource/shader_create_info.h
         foe/graphics/resource/shader.hpp
         foe/graphics/resource/type_defs.h
         foe/graphics/resource/vertex_descriptor_create_info.h
         foe/graphics/resource/vertex_descriptor.hpp)
