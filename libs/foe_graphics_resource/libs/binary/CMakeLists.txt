# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics_resource_binary)

# Declaration
add_library(foe_graphics_resource_binary SHARED)
add_library(foe::graphics::resource::binary ALIAS foe_graphics_resource_binary)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_graphics_resource_binary
                      PUBLIC foe_ecs foe_graphics_resource foe_imex_binary)

target_code_coverage(foe_graphics_resource_binary)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics_resource_binary
             PROPERTY EXPORT_NAME graphics::resource::binary)

install(
  TARGETS foe_graphics_resource_binary
  EXPORT foe_graphics_resource_binary
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics_resource_binary
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics_resource_binary-config.cmake
        DESTINATION share/cmake/foe)
