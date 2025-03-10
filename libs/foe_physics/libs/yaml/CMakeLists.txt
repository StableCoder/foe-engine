# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics_yaml)

# Declaration
add_library(foe_physics_yaml SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_physics_yaml PUBLIC foe_ecs_yaml foe_physics foe_yaml
                                              foe_imex_yaml)

target_code_coverage(foe_physics_yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_physics_yaml PROPERTY EXPORT_NAME physics_yaml)

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
