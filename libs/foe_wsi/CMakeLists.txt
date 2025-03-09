# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_wsi)

# Options
option(BUILD_WINDOW_TESTS
       "Enables building tests that utilize Window subsystems" OFF)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_wsi INTERFACE)

# Definition
add_subdirectory(include)

target_link_libraries(foe_wsi INTERFACE foe_core Vulkan::Vulkan)

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/loader)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
