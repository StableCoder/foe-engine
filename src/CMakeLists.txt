target_sources(foe_engine PRIVATE main.cpp)

target_sources(
  foe_engine_lib
  PRIVATE application.cpp
          armature_system.cpp
          camera_system.cpp
          frame_timer.cpp
          graphics.cpp
          log.cpp
          logging.cpp
          position_descriptor_pool.cpp
          settings.cpp
          vk_animation.cpp)

if(BUILD_XR)
  target_sources(foe_engine_lib PRIVATE xr.cpp)
endif()

add_subdirectory(imgui)
add_subdirectory(state_import)
add_subdirectory(state_yaml)
