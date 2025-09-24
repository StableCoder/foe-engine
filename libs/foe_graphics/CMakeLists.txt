# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics)

# Declaration
add_library(foe_graphics SHARED)
add_library(foe::graphics ALIAS foe_graphics)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_graphics PUBLIC foe_core)

target_code_coverage(foe_graphics)

if(MSVC)
  target_compile_options(foe_graphics PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics PROPERTY EXPORT_NAME graphics)

install(
  TARGETS foe_graphics
  EXPORT foe_graphics
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_graphics
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_graphics-config.cmake DESTINATION share/cmake/foe)
