# Copyright (C) 2022-2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position)

# Declaration
add_library(foe_position SHARED)
add_library(foe::position ALIAS foe_position)

# Definition
set_target_properties(foe_position PROPERTIES FOLDER "Libraries")
set_target_properties(foe_position PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_position EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/export.h)

target_link_libraries(foe_position PUBLIC foe_core foe_imex foe_simulation)

target_code_coverage(foe_position PUBLIC)

if(MSVC)
  target_compile_options(foe_position PRIVATE /EHsc)
else()
  target_compile_options(foe_position PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_position
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/export.h
         include/foe/position/component/3d.hpp
         include/foe/position/component/3d_pool.h
         include/foe/position/binary.h
         include/foe/position/compare.h
         include/foe/position/registration.h
         include/foe/position/result.h
         include/foe/position/type_defs.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  standalone_header_compile_test(foe_position "foe/position")
endif()

# install
set_property(TARGET foe_position PROPERTY EXPORT_NAME foe::position)

install(
  TARGETS foe_position
  EXPORT foe_position
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_position DESTINATION share/cmake/foe)

install(FILES foe_position-config.cmake DESTINATION share/cmake/foe)
