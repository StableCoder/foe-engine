# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_resource_imgui)

# Declaration
add_library(foe_resource_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_resource_imgui PUBLIC foe_imgui foe_resource)

target_code_coverage(foe_resource_imgui)

if(MSVC)
  target_compile_options(foe_resource_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_resource_imgui PRIVATE -fno-exceptions)
endif()

# install
set_property(TARGET foe_resource_imgui PROPERTY EXPORT_NAME resource_imgui)

install(
  TARGETS foe_resource_imgui
  EXPORT foe_resource_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_resource_imgui
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_resource_imgui-config.cmake DESTINATION share/cmake/foe)
