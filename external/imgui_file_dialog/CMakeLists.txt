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
target_sources(imgui_file_dialog PRIVATE ImGuiFileDialog/ImGuiFileDialog.cpp)

target_include_directories(imgui_file_dialog PUBLIC include ImGuiFileDialog)

target_link_libraries(imgui_file_dialog PUBLIC imgui)
