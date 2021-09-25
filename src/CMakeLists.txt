target_sources(foe_engine PRIVATE main.cpp)

target_sources(
  foe_engine_lib
  PRIVATE application.cpp
          armature_system.cpp
          camera_system.cpp
          error_code.cpp
          frame_timer.cpp
          graphics.cpp
          log.cpp
          logging.cpp
          position_descriptor_pool.cpp
          vk_animation.cpp
          window.cpp)

if(XR_SUPPORT)
  target_sources(foe_engine_lib PRIVATE xr.cpp xr_vk_camera_system.cpp)
endif()

# Exception-handled code
target_sources(
  foe_engine_lib_eh
  PRIVATE export_registrar.cpp import_registrar.cpp
          register_basic_functionality.cpp registrar.cpp settings.cpp)

add_subdirectory(imgui)
add_subdirectory(state_import)
