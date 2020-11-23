cmake_minimum_required(VERSION 3.13)
project(imgui CXX)

# Declaration
if(WIN32)
  add_library(imgui STATIC)
else()
  add_library(imgui)
endif()

# Definition
target_include_directories(imgui PUBLIC include)
if(UNIX)
  target_compile_options(imgui PRIVATE -fPIC)
endif()

add_subdirectory(src)
