# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imgui)

# Declaration
add_library(foe_imgui)
add_library(foe::imgui ALIAS foe_imgui)

# Definition
generate_export_header(foe_imgui EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/imgui/export.h)

target_link_libraries(foe_imgui PUBLIC foe_core imgui_core imgui_file_dialog)

target_code_coverage(foe_imgui)

if(MSVC)
  target_compile_options(foe_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_imgui PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_imgui
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/imgui/export.h
         include/foe/imgui/plain_old_data.hpp
         include/foe/imgui/state.hpp)

add_subdirectory(src)

# Auxiliary Target
add_subdirectory(libs/vulkan)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_imgui PROPERTY EXPORT_NAME imgui)

install(
  TARGETS foe_imgui imgui_core imgui_file_dialog
  EXPORT foe_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_imgui
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_imgui-config.cmake DESTINATION share/cmake/foe)
