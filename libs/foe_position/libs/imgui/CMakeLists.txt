# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position_imgui)

# Declaration
add_library(foe_position_imgui)
add_library(foe::position::imgui ALIAS foe_position_imgui)

# Definition
set_target_properties(foe_position_imgui PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_position_imgui EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/imgui/export.h)

target_link_libraries(foe_position_imgui PUBLIC foe_imgui foe_position
                                                foe_simulation_imgui)

target_code_coverage(foe_position_imgui)

if(MSVC)
  target_compile_options(foe_position_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_position_imgui PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_position_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/imgui/export.h
         include/foe/position/imgui/registration.hpp)

add_subdirectory(src)

# install
set_property(TARGET foe_position_imgui PROPERTY EXPORT_NAME
                                                foe::position::imgui)

install(
  TARGETS foe_position_imgui
  EXPORT foe_position_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_position_imgui DESTINATION share/cmake/foe)

install(FILES foe_position_imgui-config.cmake DESTINATION share/cmake/foe)
