# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics)

# Dependencies
find_package(Bullet REQUIRED)

# Declaration
add_library(foe_physics SHARED)
set_property(TARGET foe_physics PROPERTY EXPORT_NAME physics)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_include_directories(foe_physics
                           PUBLIC $<BUILD_INTERFACE:${BULLET_INCLUDE_DIRS}>)

target_link_libraries(foe_physics PUBLIC foe_core foe_ecs foe_imex foe_position
                                         ${BULLET_LIBRARIES})

target_code_coverage(foe_physics)

if(MSVC)
  target_compile_options(foe_physics PRIVATE /EHsc)
else()
  target_compile_options(foe_physics PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
install(
  TARGETS foe_physics
  EXPORT foe_physics
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_physics
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_physics-config.cmake DESTINATION share/cmake/foe)
