# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_vk
  PRIVATE backend.c
          binary.c
          builtin_descriptor_sets.cpp
          cleanup.c
          compare.c
          debug_callback.cpp
          descriptor_set_layout_pool.cpp
          error_colour_image.cpp
          error_depth_stencil_image.cpp
          format.c
          fragment_descriptor_pool_public.cpp
          fragment_descriptor_pool.cpp
          fragment_descriptor.cpp
          image.cpp
          log.cpp
          mesh.cpp
          model.cpp
          pipeline_pool.cpp
          queue_family.cpp
          render_graph.cpp
          render_pass_pool_public.cpp
          render_pass_pool.cpp
          render_target.cpp
          runtime.cpp
          render_view_pool.c
          result.c
          sample_count.c
          session.cpp
          shader_public.cpp
          swapchain.c
          upload_buffer_public.cpp
          upload_context_public.cpp
          upload_request_public.cpp
          upload_request.cpp
          vertex_descriptor.cpp
          vk_binary.c
          vk_result.c)

add_subdirectory(render_graph/job)
add_subdirectory(render_graph/resource)
