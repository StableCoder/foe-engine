target_sources(
  foe_graphics_vk
  PRIVATE backend.c
          debug_callback.cpp
          log.cpp
          mesh.cpp
          pipeline_pool.cpp
          resource_uploader.cpp
          runtime.cpp
          session.cpp)
