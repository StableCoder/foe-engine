target_sources(foe_bringup PRIVATE main.cpp)

target_sources(
  foe_bringup_lib
  PRIVATE application.cpp
          error_code.cpp
          frame_timer.cpp
          graphics.cpp
          log.cpp
          logging.cpp
          window.cpp
          register_basic_functionality.cpp
          #
          render_graph/render_scene.cpp)

if(XR_SUPPORT)
  target_sources(foe_bringup_lib PRIVATE xr.cpp xr_vk_camera_system.cpp)
endif()

# Exception-handled code
target_sources(foe_bringup_lib_eh PRIVATE log.cpp settings.cpp)

add_subdirectory(imgui)
add_subdirectory(simulation)
add_subdirectory(state_import)
