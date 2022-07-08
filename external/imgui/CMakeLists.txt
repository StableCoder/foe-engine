# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)
project(imgui CXX)

# Global Options
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Declarations
if(WIN32)
add_library(imgui STATIC)
else()
add_library(imgui)
endif()
add_library(imgui_demo STATIC)

# imgui Definition
target_sources(imgui PRIVATE imgui/imgui.cpp imgui/imgui_draw.cpp
                             imgui/imgui_tables.cpp imgui/imgui_widgets.cpp)

target_include_directories(imgui PUBLIC include imgui)
if(UNIX)
  target_compile_options(imgui PRIVATE -fPIC)
endif()

# imgui_demo Definition
target_sources(imgui_demo PRIVATE imgui/imgui_demo.cpp)

target_link_libraries(imgui_demo PUBLIC imgui)

if(UNIX)
  target_compile_options(imgui_demo PRIVATE -fPIC)
endif()
