# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_ecs_yaml CXX)

# Declaration
add_library(foe_ecs_yaml SHARED)
add_library(foe::ecs::yaml ALIAS foe_ecs_yaml)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_ecs_yaml PUBLIC foe_ecs foe_yaml)

target_code_coverage(foe_ecs_yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_ecs_yaml PROPERTY EXPORT_NAME ecs::yaml)

install(
  TARGETS foe_ecs_yaml
  EXPORT foe_ecs_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_ecs_yaml
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_ecs_yaml-config.cmake DESTINATION share/cmake/foe)
