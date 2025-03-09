# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_simulation_imgui)

# Declaration
add_library(foe_simulation_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_simulation_imgui PUBLIC foe_imgui foe_simulation)

target_code_coverage(foe_simulation_imgui)

if(MSVC)
  target_compile_options(foe_simulation_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_simulation_imgui PRIVATE -fno-exceptions)
endif()
