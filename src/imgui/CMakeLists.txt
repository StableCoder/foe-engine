# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_imgui
  PRIVATE armature_state.cpp
          armature.cpp
          bringup_registration.cpp
          developer_console.cpp
          entity_list.cpp
          frame_time_info.cpp
          register.cpp
          render_state.cpp
          resource_list.cpp
          save.cpp
          termination.cpp)

if(IMGUI_DEMO)
  target_sources(foe_bringup_imgui PRIVATE demo.cpp)
endif()
