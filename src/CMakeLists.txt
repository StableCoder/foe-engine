target_sources(
  foe_engine
  PRIVATE camera_descriptor_pool.cpp
          engine.cpp
          frame_timer.cpp
          logging.cpp
          settings.cpp
          vulkan_setup.cpp
          xr_setup.cpp)

add_subdirectory(imgui)
