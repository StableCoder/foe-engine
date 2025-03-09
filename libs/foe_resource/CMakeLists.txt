# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_resource)

# Declaration
add_library(foe_resource SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_resource PUBLIC foe_core foe_ecs)

target_code_coverage(foe_resource)

if(MSVC)
  target_compile_options(foe_resource PRIVATE /EHsc)
else()
  target_compile_options(foe_resource PRIVATE -fno-exceptions)
endif()

# Other
add_subdirectory(libs/imgui)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
