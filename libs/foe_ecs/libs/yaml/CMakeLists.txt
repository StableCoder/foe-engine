# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_ecs_yaml CXX)

# Declaration
add_library(foe_ecs_yaml SHARED)
add_library(foe::ecs::yaml ALIAS foe_ecs_yaml)

# Definition
set_target_properties(foe_ecs_yaml PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_ecs_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/yaml/export.h)

target_link_libraries(foe_ecs_yaml PUBLIC foe_ecs foe_yaml)

target_code_coverage(foe_ecs_yaml)

target_sources(
  foe_ecs_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/yaml/export.h
         include/foe/ecs/yaml/id.hpp
         include/foe/ecs/yaml/indexes.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_ecs_yaml PROPERTY EXPORT_NAME foe::ecs::yaml)

install(
  TARGETS foe_ecs_yaml
  EXPORT foe_ecs_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_ecs_yaml DESTINATION share/cmake/foe)

install(FILES foe_ecs_yaml-config.cmake DESTINATION share/cmake/foe)
