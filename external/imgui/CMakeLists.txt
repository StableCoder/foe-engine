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
set_property(TARGET imgui PROPERTY EXPORT_NAME foe::external::imgui)

if(UNIX)
  target_compile_options(imgui PRIVATE -fPIC)
endif()

# copy headers to custom location to avoid clashes
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/imgui.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
          ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/imgui.h
  DEPENDS imgui/imgui.h)
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/imconfig.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
          ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/imconfig.h
  DEPENDS imgui/imconfig.h)

target_sources(
  imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}/include/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/imconfig.h
         ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/imgui.h
  PRIVATE imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp
          imgui/imgui_widgets.cpp)

target_include_directories(
  imgui
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/foe/external>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>)

# imgui_demo Definition
set_property(TARGET imgui_demo PROPERTY EXPORT_NAME foe::external::imgui::demo)

if(UNIX)
  target_compile_options(imgui_demo PRIVATE -fPIC)
endif()

target_link_libraries(imgui_demo PUBLIC imgui)

target_sources(imgui_demo PRIVATE imgui/imgui_demo.cpp)
