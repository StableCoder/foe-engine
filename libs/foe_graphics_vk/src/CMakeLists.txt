target_sources(
  foe_graphics_vk
  PRIVATE backend.c
          debug_callback.cpp
          error_colour_image.cpp
          error_depth_stencil_image.cpp
          image.cpp
          log.cpp
          mesh.cpp
          model.cpp
          pipeline_pool.cpp
          resource_uploader.cpp
          runtime.cpp
          session.cpp)
