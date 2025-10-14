# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_physics_imgui)

# Declaration
add_library(foe_physics_imgui)
add_library(foe::physics::imgui ALIAS foe_physics_imgui)

# Definition
generate_export_header(foe_physics_imgui EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/imgui/export.h)

target_link_libraries(
  foe_physics_imgui PUBLIC foe_imgui foe_physics foe_resource_imgui
                           foe_simulation_imgui)

target_code_coverage(foe_physics_imgui)

if(MSVC)
  target_compile_options(foe_physics_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_physics_imgui PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_physics_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/physics/imgui/export.h
         include/foe/physics/imgui/registration.hpp)

add_subdirectory(src)

# install
set_property(TARGET foe_physics_imgui PROPERTY EXPORT_NAME foe::physics::imgui)

install(
  TARGETS foe_physics_imgui
  EXPORT foe_physics_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_physics_imgui DESTINATION share/cmake/foe)

install(FILES foe_physics_imgui-config.cmake DESTINATION share/cmake/foe)
