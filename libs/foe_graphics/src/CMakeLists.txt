target_sources(
  foe_graphics
  PRIVATE builtin_descriptor_sets.cpp
          descriptor_set_layout_pool.cpp
          environment.cpp
          fragment_descriptor.cpp
          gfx_log.cpp
          pipeline_pool.cpp
          render_pass_pool.cpp
          shader_pool.cpp
          shader.cpp
          swapchain.cpp)
