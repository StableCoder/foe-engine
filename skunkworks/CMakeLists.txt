# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(FoE-Skunkworks C CXX)

# Modify CMake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

# options
include(CMakeDependentOption)

option(EDITOR_MODE "Build with the editor UI capability" OFF)
cmake_dependent_option(
  IMGUI_DEMO "Build with the ImGui demo in the bringup app" OFF EDITOR_MODE OFF)

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

# miscellaneous
include(code-coverage)
include(formatting)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
if(NOT TARGET foe_core)
  find_package(
    foe REQUIRED
    COMPONENTS # regular
               core
               ecs
               graphics_resource
               physics
               position
               xr
               # binary imex
               graphics::resource::binary
               imex::binary
               physics::binary
               position::binary
               # yaml imex
               ecs::yaml
               imex::yaml
               graphics::resource::yaml
               physics::yaml
               position::yaml
               # imgui
               graphics::resource::imgui
               imgui::vk
               physics::imgui
               position::imgui)
  if(XR_SUPPORT)
    find_package(foe REQUIRED COMPONENTS xr::openxr xr::openxr::vk)
  endif()
endif()

find_package(ImageMagick REQUIRED MagickCore)

# Engine Entrypoint
add_executable(foe_skunkworks)
target_link_libraries(foe_skunkworks PRIVATE foe_skunkworks_lib)
target_code_coverage(
  foe_skunkworks
  OBJECTS
  foe_skunkworks_lib
  EXCLUDE
  ${CMAKE_SOURCE_DIR}/external/.*
  ARGS
  --config
  ${CMAKE_CURRENT_SOURCE_DIR}/foe-settings.yml)

# Engine Library
set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Delcarations
add_library(foe_skunkworks_lib)
add_library(foe_skunkworks_imgui)
add_library(foe_skunkworks_lib_eh)
add_library(foe_skunkworks_shared SHARED)
add_library(foe_skunkworks_binary SHARED)
add_library(foe_skunkworks_yaml SHARED)

# Sources
add_subdirectory(src)

# Engine Definition
target_link_libraries(
  foe_skunkworks_lib PUBLIC foe_skunkworks_lib_eh foe::core foe::ecs foe::imex
                            foe::external::vk_result_serialization)

# These are plugins which are linked because the bringup library is using
# header/content directly
target_link_libraries(
  foe_skunkworks_lib PUBLIC foe_skunkworks_shared foe::graphics::resource
                            foe::physics foe::position ImageMagick::MagickCore)

target_include_directories(foe_skunkworks_lib
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)

target_compile_definitions(
  foe_skunkworks_lib
  PRIVATE
    # Regular Plugins
    BRINGUP_PLUGIN="$<TARGET_FILE:foe_skunkworks_shared>"
    GRAPHICS_RESOURCE_PLUGIN="$<TARGET_FILE:foe::graphics::resource>"
    PHYSICS_PLUGIN="$<TARGET_FILE:foe::physics>"
    POSITION_PLUGIN="$<TARGET_FILE:foe::position>"
    # Yaml Plugins
    BRINGUP_YAML_PLUGIN="$<TARGET_FILE:foe_skunkworks_yaml>"
    GRAPHICS_RESOURCE_YAML_PLUGIN="$<TARGET_FILE:foe::graphics::resource::yaml>"
    IMEX_YAML_PLUGIN="$<TARGET_FILE:foe::imex::yaml>"
    PHYSICS_YAML_PLUGIN="$<TARGET_FILE:foe::physics::yaml>"
    POSITION_YAML_PLUGIN="$<TARGET_FILE:foe::position::yaml>"
    # Binary Plugin
    IMEX_BINARY_PLUGIN="$<TARGET_FILE:foe::imex::binary>"
    PHYSICS_BINARY_PLUGIN="$<TARGET_FILE:foe::physics::binary>"
    POSITION_BINARY_PLUGIN="$<TARGET_FILE:foe::position::binary>"
    GRAPHICS_RESOURCE_BINARY_PLUGIN="$<TARGET_FILE:foe::graphics::resource::binary>"
    BRINGUP_BINARY_PLUGIN="$<TARGET_FILE:foe_skunkworks_binary>")

if(MSVC)
  target_compile_options(foe_skunkworks_lib PRIVATE /EHsc)
else()
  target_compile_options(foe_skunkworks_lib PRIVATE -fno-exceptions)
endif()

if(EDITOR_MODE)
  target_link_libraries(foe_skunkworks_lib PUBLIC foe_skunkworks_imgui)
endif()

target_code_coverage(
  foe_skunkworks_lib
  OBJECTS
  foe_skunkworks_shared
  foe_skunkworks_lib_eh
  foe::core
  foe::ecs
  foe::graphics::resource
  foe::graphics::vk
  foe::imex
  foe::model
  foe::physics
  foe::position
  foe::simulation)

if(XR_SUPPORT)
  target_link_libraries(
    foe_skunkworks_lib PUBLIC foe::xr::openxr foe::xr::openxr::vk
                              foe::external::xr_result_serialization)
else()
  target_link_libraries(foe_skunkworks_lib PUBLIC foe::xr)
endif()

# Engine ImGUI Definition
target_compile_definitions(foe_skunkworks_imgui PUBLIC EDITOR_MODE)

target_link_libraries(
  foe_skunkworks_imgui
  PUBLIC foe_skunkworks_shared
         foe::imgui
         foe::imgui::vk
         foe::graphics::resource::imgui
         foe::position::imgui
         foe::physics::imgui
         foe::simulation::imgui)

if(IMGUI_DEMO)
  target_compile_definitions(foe_skunkworks_imgui PUBLIC IMGUI_SHOW_DEMO)
  target_link_libraries(foe_skunkworks_imgui PRIVATE imgui_demo)
endif()

target_code_coverage(foe_skunkworks_imgui)

if(UNIX)
  target_compile_options(foe_skunkworks_imgui PRIVATE -fPIC)
endif()

# Exception-handling Definition
target_link_libraries(foe_skunkworks_lib_eh PUBLIC foe::simulation foe::yaml)

target_code_coverage(foe_skunkworks_lib_eh)

if(UNIX)
  target_compile_options(foe_skunkworks_lib_eh PRIVATE -fPIC)
endif()

# Bringup Functionality
target_include_directories(foe_skunkworks_shared
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)
target_link_libraries(
  foe_skunkworks_shared
  PRIVATE foe::model
          foe::model::assimp
          foe::resource
          foe::simulation
          foe::position
          foe::graphics::resource
          foe::external::vk_result_serialization)

# Binary Definition
target_include_directories(foe_skunkworks_binary
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)
target_link_libraries(foe_skunkworks_binary PRIVATE foe_skunkworks_shared
                                                    foe::imex::binary)

# Yaml Definition
target_include_directories(foe_skunkworks_yaml
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public/)
target_link_libraries(
  foe_skunkworks_yaml PRIVATE foe_skunkworks_shared foe::ecs::yaml
                              foe::imex::yaml)
