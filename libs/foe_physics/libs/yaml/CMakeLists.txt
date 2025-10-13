# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics_yaml)

# Declaration
add_library(foe_physics_yaml SHARED)
add_library(foe::physics::yaml ALIAS foe_physics_yaml)

# Definition
generate_export_header(
  foe_physics_yaml EXPORT_MACRO_NAME FOE_PHYSICS_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/yaml/export.h)

target_link_libraries(foe_physics_yaml PUBLIC foe_ecs_yaml foe_physics foe_yaml
                                              foe_imex_yaml)

target_code_coverage(foe_physics_yaml)

target_sources(
  foe_physics_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/yaml/export.h
         include/foe/physics/yaml/export_registration.h
         include/foe/physics/yaml/import_registration.h
         include/foe/physics/yaml/result.h
         include/foe/physics/yaml/structs.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_physics_yaml PROPERTY EXPORT_NAME physics::yaml)

install(
  TARGETS foe_physics_yaml
  EXPORT foe_physics_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_physics_yaml
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_physics_yaml-config.cmake DESTINATION share/cmake/foe)
