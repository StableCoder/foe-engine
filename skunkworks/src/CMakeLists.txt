# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(foe_skunkworks PRIVATE main.cpp)

target_sources(
  foe_skunkworks_lib
  PRIVATE application.cpp
          frame_timer.cpp
          graphics.cpp
          log.cpp
          logging.cpp
          register_basic_functionality.cpp
          render_to_file.cpp
          result.c
          vk_result.c
          #
          render_graph/render_scene.cpp)

if(XR_SUPPORT)
  target_sources(foe_skunkworks_lib PRIVATE xr.cpp xr_result.c)
endif()

# Exception-handled code
target_sources(foe_skunkworks_lib_eh PRIVATE log.cpp settings.cpp)

add_subdirectory(imgui)
add_subdirectory(simulation)
add_subdirectory(state_import)

# wsi
add_subdirectory(wsi_glfw)
