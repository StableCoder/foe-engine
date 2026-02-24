# Copyright (C) 2022-2026 George Cave.
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
          window_surface.cpp
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
find_package(glfw3)
find_package(SDL3 COMPONENTS SDL3)

if(glfw3_FOUND)
  message(STATUS "Compiling foe_skunkworks with GLFW windowing support")

  target_compile_definitions(foe_skunkworks_lib PUBLIC FOE_SKUNKWORKS_GLFW)
  target_link_libraries(foe_skunkworks_lib PUBLIC glfw)
  target_sources(foe_skunkworks_lib PRIVATE wsi_glfw/window.cpp)

  target_compile_definitions(foe_skunkworks_imgui PUBLIC FOE_SKUNKWORKS_GLFW)
  target_link_libraries(foe_skunkworks_imgui PUBLIC glfw)
  target_sources(foe_skunkworks_imgui PRIVATE wsi_glfw/imgui.cpp)
endif()
if(SDL3_FOUND)
  message(STATUS "Compiling foe_skunkworks with SDL3 windowing support")

  target_compile_definitions(foe_skunkworks_lib PUBLIC FOE_SKUNKWORKS_SDL3)
  target_link_libraries(foe_skunkworks_lib PUBLIC SDL3::SDL3)
  target_sources(foe_skunkworks_lib PRIVATE wsi_sdl3/window.cpp)

  target_compile_definitions(foe_skunkworks_imgui PUBLIC FOE_SKUNKWORKS_SDL3)
  target_link_libraries(foe_skunkworks_imgui PUBLIC SDL3::SDL3)
  target_sources(foe_skunkworks_imgui PRIVATE wsi_sdl3/imgui.cpp)
endif()
if(Qt6_FOUND)
  message(STATUS "Compiling foe_skunkworks with Qt6 windowing support")

  target_compile_definitions(foe_skunkworks_lib PUBLIC FOE_SKUNKWORKS_QT)
  target_link_libraries(foe_skunkworks_lib PUBLIC Qt6::Gui Qt6::GuiPrivate)
  target_sources(foe_skunkworks_lib PRIVATE wsi_qt/qt_vulkan_window.cpp
                                            wsi_qt/window.cpp)

  if(LINUX)
    target_sources(foe_skunkworks_lib PRIVATE wsi_qt/surface_wayland.cpp
                                              wsi_qt/surface_xcb.cpp)
  elseif(WIN32)
    target_sources(foe_skunkworks_lib PRIVATE wsi_qt/surface_win32.cpp)
  endif()

  target_compile_definitions(foe_skunkworks_imgui PUBLIC FOE_SKUNKWORKS_QT)
  target_link_libraries(foe_skunkworks_imgui PUBLIC Qt6::Gui)
  target_sources(foe_skunkworks_imgui PRIVATE wsi_qt/imgui.cpp)
endif()
