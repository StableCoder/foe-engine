# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Modify CMake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

cmake_minimum_required(VERSION 3.23)
include(afl-fuzzing)
project(FoE-Engine C CXX)

# Global Options
include(CMakeDependentOption)
option(BUILD_SHADERS "Build GLSL shaders to SPIR-V automatically" ON)
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_VULKAN_RUNTIME_TESTS
       "Build tests that require an available Vulkan runtime" OFF)

# There are cases where it is easier to debug issues if dynamic plugin library
# content is still loaded into the application when it terminates, such as for
# tools that do post-analysis such as sanitizers.
option(DISABLE_PLUGIN_UNLOAD "Prevent plugins from unloading" OFF)

option(EDITOR_MODE "Build with the editor UI capability" OFF)
cmake_dependent_option(
  IMGUI_DEMO "Build with the ImGui demo in the bringup app" OFF EDITOR_MODE OFF)

# XR Settings
if(APPLE)
  option(XR_SUPPORT "Compile/link XR device support" OFF)
else()
  option(XR_SUPPORT "Compile/link XR device support" ON)
endif()

# C11 standard, no extensions
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# C++20 standard, no extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Code Coverage
include(code-coverage)
add_code_coverage_all_targets(EXCLUDE /usr/.* .*/test/.*
                              ${CMAKE_CURRENT_SOURCE_DIR}/external/.*)

# Formatting
include(formatting)
file(
  GLOB_RECURSE
  CL_FILES
  *.[hc]
  *.[hc]pp
  *.vert
  *.tese
  *.tesc
  *.geom
  *.frag
  *.comp)
list(FILTER CL_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
list(FILTER CL_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(FILTER CL_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/external)
clang_format(format ${CL_FILES})

file(GLOB_RECURSE CM_FILES CMakeLists.txt *.cmake)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/external)
cmake_format(cmake-format ${CM_FILES})

# Dependency Graph
include(dependency-graph)
gen_dep_graph(png ADD_TO_DEP_GRAPH)

# clang-tidy
include(tools)
option(CLANG_TIDY "Run clang-tidy static analysis" OFF)
cmake_dependent_option(
  CLANG_TIDY_FIX "Turns on fixes for found clang-tidy issues" OFF CLANG_TIDY
  OFF)
option(CLANG_TIDY_FIX "" OFF)
if(CLANG_TIDY_FIX)
  clang_tidy(-header-filter='${CMAKE_SOURCE_DIR}/*' -fix)
elseif(CLANG_TIDY)
  clang_tidy(-header-filter='${CMAKE_SOURCE_DIR}/*')
endif()

# Header Exports
include(GenerateExportHeader)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Misc CMake
include(compiler-options)
include(glsl-shaders)
include(sanitizers)

if(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()
if(BUILD_TESTS)
  enable_testing()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW/Vulkan
add_compile_definitions(GLFW_INCLUDE_VULKAN)

# Libraries
add_subdirectory(external)
add_subdirectory(libs)

# install
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/foe-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/foe-config.cmake
  INSTALL_DESTINATION cmake
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/foe-config.cmake
        DESTINATION share/cmake/foe)
