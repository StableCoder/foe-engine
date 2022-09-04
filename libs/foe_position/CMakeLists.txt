# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_position)

# Dependencies
find_package(glm REQUIRED)

# Declaration
add_library(foe_position SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_position EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/position/export.h)

target_include_directories(foe_position
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_position PUBLIC foe foe_imex foe_simulation glm)

target_code_coverage(foe_position PUBLIC)

if(MSVC)
  target_compile_options(foe_position PRIVATE /EHsc)
else()
  target_compile_options(foe_position PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)
