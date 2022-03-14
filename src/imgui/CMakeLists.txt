target_sources(
  foe_engine_imgui
  PRIVATE armature_state.cpp
          bringup_registration.cpp
          camera.cpp
          developer_console.cpp
          entity_list.cpp
          frame_time_info.cpp
          register.cpp
          render_state.cpp
          resource_list.cpp
          save.cpp
          termination.cpp)

if(IMGUI_DEMO)
  target_sources(foe_engine_imgui PRIVATE demo.cpp)
endif()
