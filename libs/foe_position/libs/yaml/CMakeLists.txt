# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position_yaml)

# Declaration
add_library(foe_position_yaml SHARED)
add_library(foe::position::yaml ALIAS foe_position_yaml)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_position_yaml PUBLIC foe_imex_yaml foe_position)

target_code_coverage(foe_position_yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_position_yaml PROPERTY EXPORT_NAME position::yaml)

install(
  TARGETS foe_position_yaml
  EXPORT foe_position_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_position_yaml
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_position_yaml-config.cmake DESTINATION share/cmake/foe)
