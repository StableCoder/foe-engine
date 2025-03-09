# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_xr_openxr_vk)

# Declaration
add_library(foe_xr_openxr_vk)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_xr_openxr_vk PUBLIC foe_xr_openxr foe_graphics_vk)

target_code_coverage(foe_xr_openxr_vk)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
