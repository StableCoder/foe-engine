target_sources(
  foe_graphics
  PRIVATE builtin_descriptor_sets.cpp
          descriptor_set_layout_pool.cpp
          environment.cpp
          error_colour_image.cpp
          error_depth_stencil_image.cpp
          fragment_descriptor_pool.cpp
          fragment_descriptor.cpp
          gfx_log.cpp
          image.cpp
          pipeline_pool.cpp
          render_pass_pool.cpp
          resource_uploader.cpp
          shader_pool.cpp
          shader.cpp
          swapchain.cpp
          upload_data.cpp)
