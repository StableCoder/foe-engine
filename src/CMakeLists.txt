target_sources(
  foe_engine
  PRIVATE camera_descriptor_pool.cpp
          engine_settings.cpp
          engine.cpp
          frame_timer.cpp
          logging.cpp
          vulkan_setup.cpp
          xr_setup.cpp)

add_subdirectory(imgui)
