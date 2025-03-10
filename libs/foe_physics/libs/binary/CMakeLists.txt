# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics_binary)

# Declaration
add_library(foe_physics_binary SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_physics_binary PUBLIC foe_ecs foe_physics
                                                foe_imex_binary)

target_code_coverage(foe_physics_binary)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_physics_binary PROPERTY EXPORT_NAME physics_binary)

install(
  TARGETS foe_physics_binary
  EXPORT foe_physics_binary
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_physics_binary
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_physics_binary-config.cmake DESTINATION share/cmake/foe)
