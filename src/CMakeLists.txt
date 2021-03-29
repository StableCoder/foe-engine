target_sources(
  foe_engine
  PRIVATE application.cpp
          camera_descriptor_pool.cpp
          frame_timer.cpp
          graphics.cpp
          logging.cpp
          main.cpp
          position_descriptor_pool.cpp
          settings.cpp
          vk_animation.cpp)

if(BUILD_XR)
  target_sources(foe_engine PRIVATE xr.cpp)
endif()

add_subdirectory(imgui)
add_subdirectory(state_yaml)
