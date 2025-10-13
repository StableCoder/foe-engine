# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position_binary)

# Declaration
add_library(foe_position_binary SHARED)
add_library(foe::position::binary ALIAS foe_position_binary)

# Definition
generate_export_header(foe_position_binary EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/binary/export.h)

target_link_libraries(foe_position_binary PUBLIC foe_ecs foe_position
                                                 foe_imex_binary)

target_code_coverage(foe_position_binary)

target_sources(
  foe_position_binary
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/binary/export.h
         include/foe/position/binary/export_registration.h
         include/foe/position/binary/import_registration.h
         include/foe/position/binary/result.h)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_position_binary PROPERTY EXPORT_NAME position::binary)

install(
  TARGETS foe_position_binary
  EXPORT foe_position_binary
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_position_binary
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_position_binary-config.cmake DESTINATION share/cmake/foe)
