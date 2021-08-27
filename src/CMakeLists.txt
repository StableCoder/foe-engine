target_sources(foe_engine PRIVATE main.cpp)

target_sources(
  foe_engine_lib
  PRIVATE application.cpp
          armature_system.cpp
          camera_system.cpp
          error_code.cpp
          export_registrar.cpp
          frame_timer.cpp
          graphics.cpp
          import_registrar.cpp
          log.cpp
          logging.cpp
          position_descriptor_pool.cpp
          register_basic_functionality.cpp
          registrar.cpp
          settings.cpp
          vk_animation.cpp
          window.cpp)

if(BUILD_XR)
  target_sources(foe_engine_lib PRIVATE xr.cpp xr_vk_camera_system.cpp)
endif()

add_subdirectory(imgui)
add_subdirectory(state_import)
