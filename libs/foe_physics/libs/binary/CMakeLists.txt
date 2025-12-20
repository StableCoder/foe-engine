# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics_binary)

# Declaration
add_library(foe_physics_binary SHARED)
add_library(foe::physics::binary ALIAS foe_physics_binary)

# Definition
set_target_properties(foe_physics_binary PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_physics_binary EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/binary/export.h)

target_link_libraries(foe_physics_binary PUBLIC foe_ecs foe_physics
                                                foe_imex_binary)

target_code_coverage(foe_physics_binary)

target_sources(
  foe_physics_binary
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/binary/export.h
         include/foe/physics/binary/export_registration.h
         include/foe/physics/binary/import_registration.h
         include/foe/physics/binary/result.h)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_physics_binary PROPERTY EXPORT_NAME
                                                foe::physics::binary)

install(
  TARGETS foe_physics_binary
  EXPORT foe_physics_binary
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_physics_binary DESTINATION share/cmake/foe)

install(FILES foe_physics_binary-config.cmake DESTINATION share/cmake/foe)
