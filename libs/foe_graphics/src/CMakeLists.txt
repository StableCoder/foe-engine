target_sources(
  foe_graphics
  PRIVATE builtin_descriptor_sets.cpp
          descriptor_set_layout_pool.cpp
          fragment_descriptor_pool.cpp
          fragment_descriptor.cpp
          gfx_log.cpp
          render_pass_pool.cpp
          shader_pool.cpp
          shader.cpp
          swapchain.cpp
          upload_data.cpp)
