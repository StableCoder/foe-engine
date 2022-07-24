# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Modify CMake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.15)
include(afl-fuzzing)
project(FoE-Engine C CXX)

# Global Options
include(CMakeDependentOption)
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_EXTRAS "Builds other programs, such as benchmarks" OFF)
option(BUILD_SHADERS "Build GLSL shaders to SPIR-V automatically" ON)
option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_VULKAN_RUNTIME_TESTS
       "Build tests that require an available Vulkan runtime" OFF)
option(EDITOR_MODE "Build with the editor UI capability" OFF)
cmake_dependent_option(
  IMGUI_DEMO "Build with the ImGui demo in the bringup app" OFF EDITOR_MODE OFF)

# WSI Loader/Implementation Settings
option(
  WSI_LOADER
  "Compile/link the WSI loader library (implementations are shared objects)" ON)
set(WSI_LIBRARY
    "glfw3"
    CACHE
      STRING
      "WSI library linked/loaded by default (passed via compile definitions)")

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
option(CLANG_TIDY_FIX "" OFF)
if(CLANG_TIDY_FIX)
  clang_tidy(-header-filter='${CMAKE_SOURCE_DIR}/*' -fix)
else()
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
include(prepare-catch)
include(sanitizers)

if(WIN32)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
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

# Engine Entrypoint
add_executable(foe_bringup)
target_link_libraries(foe_bringup PRIVATE foe_bringup_lib)
target_code_coverage(foe_bringup OBJECTS foe_bringup_lib ARGS --config
                     ${CMAKE_CURRENT_SOURCE_DIR}/foe-settings.yml)

# Engine Library
set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Delcarations
add_library(foe_bringup_lib)
add_library(foe_bringup_imgui)
add_library(foe_bringup_lib_eh)
add_library(foe_bringup_shared SHARED)
add_library(foe_bringup_yaml SHARED)

# Sources
add_subdirectory(src)

# Engine Definition
target_link_libraries(
  foe_bringup_lib
  PUBLIC foe
         foe_ecs
         foe_bringup_shared
         foe_bringup_lib_eh
         foe_graphics_resource
         foe_graphics_vk
         foe_imex
         foe_physics
         foe_position)

target_include_directories(foe_bringup_lib
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)
target_compile_definitions(
  foe_bringup_lib
  PRIVATE IMEX_YAML_LIB="$<TARGET_FILE:foe_imex_yaml>"
          PHYSICS_LIB="$<TARGET_FILE:foe_physics_yaml>"
          POSITION_LIB="$<TARGET_FILE:foe_position_yaml>"
          GRAPHICS_RESOURCE_LIB="$<TARGET_FILE:foe_graphics_resource_yaml>"
          ENGINE_YAML="$<TARGET_FILE:foe_bringup_yaml>")

if(MSVC)
  target_compile_options(foe_bringup_lib PRIVATE /EHsc)
else()
  target_compile_options(foe_bringup_lib PRIVATE -fno-exceptions)
endif()

if(EDITOR_MODE)
  target_link_libraries(foe_bringup_lib PUBLIC foe_bringup_imgui)
endif()

target_code_coverage(
  foe_bringup_lib
  OBJECTS
  foe
  foe_ecs
  foe_bringup_shared
  foe_bringup_lib_eh
  foe_graphics_resource
  foe_graphics_vk
  foe_imex
  foe_model
  foe_physics
  foe_position
  foe_simulation)

if(WSI_LOADER)
  target_link_libraries(foe_bringup_lib PUBLIC foe_wsi_loader)
else()
  target_link_libraries(foe_bringup_lib PUBLIC foe_wsi_${WSI_LIBRARY})
endif()

if(XR_SUPPORT)
  target_link_libraries(foe_bringup_lib PUBLIC foe_xr_openxr foe_xr_openxr_vk)
else()
  target_link_libraries(foe_bringup_lib PUBLIC foe_xr)
endif()

# Engine ImGUI Definition
target_compile_definitions(foe_bringup_imgui PUBLIC EDITOR_MODE)

target_link_libraries(
  foe_bringup_imgui
  PUBLIC foe_bringup_shared
         foe_imgui
         foe_imgui_vk
         foe_graphics_resource_imgui
         foe_position_imgui
         foe_physics_imgui
         foe_simulation_imgui
         foe_wsi_imgui)

if(IMGUI_DEMO)
  target_compile_definitions(foe_bringup_imgui PUBLIC IMGUI_SHOW_DEMO)
  target_link_libraries(foe_bringup_imgui PRIVATE imgui_demo)
endif()

target_code_coverage(foe_bringup_imgui)

if(UNIX)
  target_compile_options(foe_bringup_imgui PRIVATE -fPIC)
endif()

# Exception-handling Definition
target_link_libraries(foe_bringup_lib_eh PUBLIC CLI11 foe_simulation foe_yaml)

target_code_coverage(foe_bringup_lib_eh)

if(UNIX)
  target_compile_options(foe_bringup_lib_eh PRIVATE -fPIC)
endif()

# Bringup Functionality
target_include_directories(foe_bringup_shared
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)
target_link_libraries(
  foe_bringup_shared PRIVATE foe_model foe_model_assimp foe_resource
                             foe_simulation foe_position foe_graphics_resource)

# Yaml Definition
target_include_directories(foe_bringup_yaml
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)
target_link_libraries(foe_bringup_yaml PRIVATE foe_bringup_shared foe_ecs_yaml
                                               foe_imex_yaml)

# Auxiliary Items
if(BUILD_SHADERS)
  file(
    GLOB_RECURSE
    GLSL_FILES
    data/*.vert
    data/*.tese
    data/*.tesc
    data/*.geom
    data/*.frag
    data/*.comp)

  target_glsl_shaders(foe_bringup PRIVATE ${GLSL_FILES})
  clang_format(format_shaders ${GLSL_FILES})
endif()
