# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_physics)

# Dependencies
find_package(Bullet REQUIRED)

# Declaration
add_library(foe_physics SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_physics EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/physics/export.h)

target_include_directories(
  foe_physics PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                     ${BULLET_INCLUDE_DIRS})

target_link_libraries(foe_physics PUBLIC foe foe_ecs foe_imex foe_position
                                         ${BULLET_LIBRARIES})

target_code_coverage(foe_physics)

if(MSVC)
  target_compile_options(foe_physics PRIVATE /EHsc)
else()
  target_compile_options(foe_physics PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
