# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_ecs)

# Declaration
add_library(foe_ecs SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_ecs PUBLIC foe_core)

target_code_coverage(foe_ecs)

if(MSVC)
  target_compile_options(foe_ecs PRIVATE /EHsc)
else()
  target_compile_options(foe_ecs PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/yaml)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
