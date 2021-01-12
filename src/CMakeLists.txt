target_sources(
  foe_engine
  PRIVATE application.cpp
          camera_descriptor_pool.cpp
          frame_timer.cpp
          graphics.cpp
          logging.cpp
          main.cpp
          settings.cpp)

if(BUILD_XR)
  target_sources(foe_engine PRIVATE xr.cpp)
endif()

add_subdirectory(imgui)
