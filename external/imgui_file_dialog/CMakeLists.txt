# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)
project(imgui_file_dialog CXX)

# Global Options
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Declarations
if(WIN32)
  add_library(imgui_file_dialog STATIC)
else()
  add_library(imgui_file_dialog)
endif()

# Definition
target_include_directories(
  imgui_file_dialog
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog>)

target_link_libraries(imgui_file_dialog PUBLIC imgui_core)

target_sources(
  imgui_file_dialog
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog/ImGuiFileDialog.h
         ${CMAKE_CURRENT_SOURCE_DIR}/include/ImGuiFileDialogConfig.h
  PRIVATE ImGuiFileDialog/ImGuiFileDialog.cpp)
