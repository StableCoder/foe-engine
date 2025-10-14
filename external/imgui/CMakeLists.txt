# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

# Global Options
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Declarations
if(WIN32)
  add_library(imgui STATIC)
else()
  add_library(imgui)
endif()
add_library(foe::external::imgui ALIAS imgui)

add_library(imgui_demo STATIC)
add_library(foe::external::imgui::demo ALIAS imgui_demo)

# imgui Definition
set_property(TARGET imgui PROPERTY EXPORT_NAME
                                               foe::external::imgui)

if(UNIX)
  target_compile_options(imgui PRIVATE -fPIC)
endif()

target_sources(
  imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/imgui
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         imgui/imgui.h
         include/imconfig.h
  PRIVATE imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp
          imgui/imgui_widgets.cpp)

# imgui_demo Definition
set_property(TARGET imgui_demo PROPERTY EXPORT_NAME
                                               foe::external::imgui::demo)

                                               if(UNIX)
  target_compile_options(imgui_demo PRIVATE -fPIC)
endif()

target_link_libraries(imgui_demo PUBLIC imgui)

target_sources(imgui_demo PRIVATE imgui/imgui_demo.cpp)
