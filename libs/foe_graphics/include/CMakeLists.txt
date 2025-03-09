# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h)

target_sources(
  foe_graphics
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h
         foe/graphics/backend.h
         foe/graphics/builtin_descriptor_sets.h
         foe/graphics/delayed_caller.h
         foe/graphics/mesh.h
         foe/graphics/render_target.h
         foe/graphics/render_view_pool.h
         foe/graphics/result.h
         foe/graphics/runtime.h
         foe/graphics/session.h
         foe/graphics/shader.h
         foe/graphics/type_defs.h
         foe/graphics/upload_buffer.h
         foe/graphics/upload_context.h
         foe/graphics/upload_request.h)
