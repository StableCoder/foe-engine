# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_graphics_vk EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h)

target_sources(
  foe_graphics_vk
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h
         foe/graphics/vk/render_graph/job/blit_image.hpp
         foe/graphics/vk/render_graph/job/copy_image.hpp
         foe/graphics/vk/render_graph/job/export_image.hpp
         foe/graphics/vk/render_graph/job/import_image.hpp
         foe/graphics/vk/render_graph/job/present_image.hpp
         foe/graphics/vk/render_graph/job/resolve_image.hpp
         foe/graphics/vk/render_graph/job/synchronize.hpp
         foe/graphics/vk/render_graph/resource/image.hpp
         foe/graphics/vk/binary.h
         foe/graphics/vk/cleanup.h
         foe/graphics/vk/compare.h
         foe/graphics/vk/error_images.h
         foe/graphics/vk/format.h
         foe/graphics/vk/fragment_descriptor.h
         foe/graphics/vk/fragment_descriptor_pool.h
         foe/graphics/vk/image.h
         foe/graphics/vk/mesh.h
         foe/graphics/vk/model.h
         foe/graphics/vk/pipeline_pool.h
         foe/graphics/vk/queue_family.h
         foe/graphics/vk/render_graph.hpp
         foe/graphics/vk/render_pass_pool.h
         foe/graphics/vk/render_target.h
         foe/graphics/vk/render_view_pool.h
         foe/graphics/vk/result.h
         foe/graphics/vk/runtime.h
         foe/graphics/vk/sample_count.h
         foe/graphics/vk/session.h
         foe/graphics/vk/shader.h
         foe/graphics/vk/swapchain.h
         foe/graphics/vk/vertex_descriptor.h
         foe/graphics/vk/vk_binary.h)
