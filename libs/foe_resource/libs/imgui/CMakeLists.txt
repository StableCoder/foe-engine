# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_resource_imgui)

# Declaration
add_library(foe_resource_imgui)
add_library(foe::resource::imgui ALIAS foe_resource_imgui)

# Definition
generate_export_header(
  foe_resource_imgui EXPORT_MACRO_NAME FOE_RES_IMGUI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/resource/imgui/export.h)

target_link_libraries(foe_resource_imgui PUBLIC foe_imgui foe_resource)

target_code_coverage(foe_resource_imgui)

if(MSVC)
  target_compile_options(foe_resource_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_resource_imgui PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_resource_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/resource/imgui/export.h
         include/foe/resource/imgui/create_info.h
         include/foe/resource/imgui/resource.h)

add_subdirectory(src)

# install
set_property(TARGET foe_resource_imgui PROPERTY EXPORT_NAME
                                                foe::resource::imgui)

install(
  TARGETS foe_resource_imgui
  EXPORT foe_resource_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_resource_imgui DESTINATION share/cmake/foe)

install(FILES foe_resource_imgui-config.cmake DESTINATION share/cmake/foe)
