# Copyright (C) 2022-2026 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_model)

# Declarations
add_library(foe_model)
add_library(foe::model ALIAS foe_model)

# Library Definition
set_target_properties(foe_model PROPERTIES FOLDER "Libraries")
set_target_properties(foe_model PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_model EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/model/export.h)

target_link_libraries(foe_model PUBLIC foe_core)

target_code_coverage(foe_model)

if(UNIX)
  target_compile_options(foe_model PRIVATE -fPIC)
endif()

if(MSVC)
  target_compile_options(foe_model PRIVATE /EHsc)
else()
  target_compile_options(foe_model PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_model
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/model/export.h
         include/foe/model/animation.hpp
         include/foe/model/armature.hpp
         include/foe/model/cube.hpp
         include/foe/model/ico_sphere.hpp
         include/foe/model/importer.hpp
         include/foe/model/vertex_component.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  standalone_header_compile_test(foe_model "foe/model")
endif()

# install
set_property(TARGET foe_model PROPERTY EXPORT_NAME foe::model)

install(
  TARGETS foe_model
  EXPORT foe_model
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_model DESTINATION share/cmake/foe)

install(FILES foe_model-config.cmake DESTINATION share/cmake/foe)
