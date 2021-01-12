target_sources(
  foe_engine
  PRIVATE application.cpp
          camera_descriptor_pool.cpp
          frame_timer.cpp
          graphics.cpp
          logging.cpp
          main.cpp
          settings.cpp
          xr.cpp)

add_subdirectory(imgui)
