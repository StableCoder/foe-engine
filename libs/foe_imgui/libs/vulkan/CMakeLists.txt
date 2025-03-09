# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imgui_vk)

# Dependencies
find_package(glfw3 REQUIRED)

# Declaration
add_library(foe_imgui_vk)
set_property(TARGET foe_imgui_vk PROPERTY EXPORT_NAME imgui_vk)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_imgui_vk PUBLIC foe_graphics_vk foe_imgui glfw)

target_code_coverage(foe_imgui_vk)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
