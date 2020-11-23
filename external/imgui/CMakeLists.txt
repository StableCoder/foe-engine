cmake_minimum_required(VERSION 3.13)
project(imgui CXX)

# Declaration
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(imgui)

# Definition
target_include_directories(imgui PUBLIC include)
if(UNIX)
  target_compile_options(imgui PRIVATE -fPIC)
endif()

add_subdirectory(src)
