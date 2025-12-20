# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imex_yaml)

# Declaration
add_library(foe_imex_yaml SHARED)
add_library(foe::imex::yaml ALIAS foe_imex_yaml)

# Definition
set_target_properties(foe_imex_yaml PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_imex_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/yaml/export.h)

target_link_libraries(foe_imex_yaml PUBLIC foe_imex foe_yaml foe_ecs_yaml
                                           foe_simulation)

target_code_coverage(foe_imex_yaml)

target_sources(
  foe_imex_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/imex/yaml/export.h
         include/foe/imex/yaml/exporter.hpp
         include/foe/imex/yaml/exporter_registration.h
         include/foe/imex/yaml/importer.hpp
         include/foe/imex/yaml/importer_registration.h
         include/foe/imex/yaml/result.h
         include/foe/imex/yaml/type_defs.h)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_imex_yaml PROPERTY EXPORT_NAME foe::imex::yaml)

install(
  TARGETS foe_imex_yaml
  EXPORT foe_imex_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_imex_yaml DESTINATION share/cmake/foe)

install(FILES foe_imex_yaml-config.cmake DESTINATION share/cmake/foe)
