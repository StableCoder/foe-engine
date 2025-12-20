# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_resource)

# Declaration
add_library(foe_resource SHARED)
add_library(foe::resource ALIAS foe_resource)

# Definition
set_target_properties(foe_resource PROPERTIES DEBUG_POSTFIX "_d")

generate_export_header(
  foe_resource EXPORT_MACRO_NAME FOE_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/resource/export.h)

target_link_libraries(foe_resource PUBLIC foe_core foe_ecs)

target_code_coverage(foe_resource)

if(MSVC)
  target_compile_options(foe_resource PRIVATE /EHsc)
else()
  target_compile_options(foe_resource PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_resource
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/resource/export.h
         include/foe/resource/create_info.h
         include/foe/resource/pool.h
         include/foe/resource/resource_fns.h
         include/foe/resource/resource.h
         include/foe/resource/result.h
         include/foe/resource/type_defs.h)

add_subdirectory(src)

# Other
add_subdirectory(libs/imgui)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_resource PROPERTY EXPORT_NAME foe::resource)

install(
  TARGETS foe_resource
  EXPORT foe_resource
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_resource DESTINATION share/cmake/foe)

install(FILES foe_resource-config.cmake DESTINATION share/cmake/foe)
