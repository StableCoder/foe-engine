# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics)

# Dependencies
find_package(Bullet REQUIRED)

# Declaration
add_library(foe_physics SHARED)
add_library(foe::physics ALIAS foe_physics)

# Definition
set_target_properties(foe_physics PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_physics EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/export.h)

target_include_directories(foe_physics PUBLIC ${BULLET_INCLUDE_DIRS})

target_link_libraries(foe_physics PUBLIC foe_core foe_ecs foe_imex foe_position
                                         ${BULLET_LIBRARIES})

target_code_coverage(foe_physics)

if(MSVC)
  target_compile_options(foe_physics PRIVATE /EHsc)
else()
  target_compile_options(foe_physics PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_physics
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/export.h
         include/foe/physics/component/rigid_body.h
         include/foe/physics/component/rigid_body_pool.h
         include/foe/physics/resource/collision_shape_create_info.hpp
         include/foe/physics/resource/collision_shape.hpp
         include/foe/physics/binary.h
         include/foe/physics/compare.h
         include/foe/physics/registration.h
         include/foe/physics/result.h
         include/foe/physics/system.h
         include/foe/physics/type_defs.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_physics PROPERTY EXPORT_NAME foe::physics)

install(
  TARGETS foe_physics
  EXPORT foe_physics
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_physics DESTINATION share/cmake/foe)

install(FILES foe_physics-config.cmake DESTINATION share/cmake/foe)
