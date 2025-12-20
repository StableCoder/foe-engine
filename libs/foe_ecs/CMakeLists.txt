# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_ecs)

# Declaration
add_library(foe_ecs SHARED)
add_library(foe::ecs ALIAS foe_ecs)

# Definition
set_target_properties(foe_ecs PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(foe_ecs EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/export.h)

target_link_libraries(foe_ecs PUBLIC foe_core)

target_code_coverage(foe_ecs)

if(MSVC)
  target_compile_options(foe_ecs PRIVATE /EHsc)
else()
  target_compile_options(foe_ecs PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_ecs
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/ecs/export.h
         include/foe/ecs/binary.h
         include/foe/ecs/component_pool.h
         include/foe/ecs/entity_list.h
         include/foe/ecs/group_translator.h
         include/foe/ecs/id.h
         include/foe/ecs/id_to_string.hpp
         include/foe/ecs/indexes.h
         include/foe/ecs/name_map.h
         include/foe/ecs/result.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_ecs PROPERTY EXPORT_NAME foe::ecs)

install(
  TARGETS foe_ecs
  EXPORT foe_ecs
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_ecs DESTINATION share/cmake/foe)

install(FILES foe_ecs-config.cmake DESTINATION share/cmake/foe)
