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
add_library(foe::external::imgui::file_dialog ALIAS imgui_file_dialog)

# Definition
set_property(TARGET imgui_file_dialog PROPERTY EXPORT_NAME
                                               foe::external::imgui::file_dialog)

target_link_libraries(imgui_file_dialog PUBLIC imgui)

target_sources(
  imgui_file_dialog
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ImGuiFileDialog/ImGuiFileDialog.h
         include/ImGuiFileDialogConfig.h
  PRIVATE ImGuiFileDialog/ImGuiFileDialog.cpp)
