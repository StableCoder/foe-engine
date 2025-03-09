# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_resource_yaml)

# Declaration
add_library(foe_graphics_resource_yaml SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(
  foe_graphics_resource_yaml PUBLIC foe_ecs_yaml foe_graphics_resource
                                    foe_graphics_vk_yaml foe_imex_yaml)

target_code_coverage(foe_graphics_resource_yaml)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
