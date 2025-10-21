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
set_property(TARGET imgui_file_dialog
             PROPERTY EXPORT_NAME foe::external::imgui::file_dialog)

target_compile_definitions(imgui_file_dialog
                           PUBLIC IMGUI_INCLUDE="foe/external/imgui.h")

target_link_libraries(imgui_file_dialog PUBLIC imgui)

# copy headers to custom location to avoid clashes
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/ImGuiFileDialog.h
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog/ImGuiFileDialog.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/ImGuiFileDialog.h
  DEPENDS ImGuiFileDialog/ImGuiFileDialog.h)
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/ImGuiFileDialogConfig.h
  COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog/ImGuiFileDialogConfig.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/ImGuiFileDialogConfig.h
  DEPENDS ImGuiFileDialog/ImGuiFileDialogConfig.h)

target_sources(
  imgui_file_dialog
  PUBLIC
    FILE_SET
    HEADERS
    BASE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}/include
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/ImGuiFileDialog.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/ImGuiFileDialogConfig.h
  PRIVATE ImGuiFileDialog/ImGuiFileDialog.cpp)
