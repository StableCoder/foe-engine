# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imgui)

# Declaration
add_library(foe_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_imgui PUBLIC foe_wsi $<BUILD_INTERFACE:imgui>
                                       $<BUILD_INTERFACE:imgui_file_dialog>)

target_code_coverage(foe_imgui)

if(MSVC)
  target_compile_options(foe_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_imgui PRIVATE -fno-exceptions)
endif()

# Auxiliary Target
add_subdirectory(libs/vulkan)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_imgui PROPERTY EXPORT_NAME imgui)

install(
  TARGETS foe_imgui
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
