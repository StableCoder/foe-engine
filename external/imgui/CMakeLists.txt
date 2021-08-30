cmake_minimum_required(VERSION 3.13)
project(imgui CXX)

# Declaration
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(imgui)

# Definition
target_sources(imgui PRIVATE imgui/imgui.cpp imgui/imgui_draw.cpp
                             imgui/imgui_tables.cpp imgui/imgui_widgets.cpp)

target_include_directories(imgui PUBLIC include imgui)
if(UNIX)
  target_compile_options(imgui PRIVATE -fPIC)
endif()
