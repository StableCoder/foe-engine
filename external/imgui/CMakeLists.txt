# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

# Global Options
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Declarations
if(WIN32)
  add_library(imgui_core STATIC)
else()
  add_library(imgui_core)
endif()
add_library(imgui_demo STATIC)

# imgui Definition
target_include_directories(
  imgui_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>)
if(UNIX)
  target_compile_options(imgui_core PRIVATE -fPIC)
endif()

target_sources(
  imgui_core
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/imgui
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/imconfig.h
  PRIVATE imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp
          imgui/imgui_widgets.cpp)

# imgui_demo Definition
target_sources(imgui_demo PRIVATE imgui/imgui_demo.cpp)

target_link_libraries(imgui_demo PUBLIC imgui_core)

if(UNIX)
  target_compile_options(imgui_demo PRIVATE -fPIC)
endif()
