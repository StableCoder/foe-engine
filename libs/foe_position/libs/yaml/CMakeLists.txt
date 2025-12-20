# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position_yaml)

# Declaration
add_library(foe_position_yaml SHARED)
add_library(foe::position::yaml ALIAS foe_position_yaml)

# Definition
set_target_properties(foe_position_yaml PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_position_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/yaml/export.h)

target_link_libraries(foe_position_yaml PUBLIC foe_imex_yaml foe_position)

target_code_coverage(foe_position_yaml)

target_sources(
  foe_position_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/yaml/export.h
         include/foe/position/yaml/export_registration.h
         include/foe/position/yaml/import_registration.h
         include/foe/position/yaml/result.h
         include/foe/position/yaml/structs.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_position_yaml PROPERTY EXPORT_NAME foe::position::yaml)

install(
  TARGETS foe_position_yaml
  EXPORT foe_position_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_position_yaml DESTINATION share/cmake/foe)

install(FILES foe_position_yaml-config.cmake DESTINATION share/cmake/foe)
