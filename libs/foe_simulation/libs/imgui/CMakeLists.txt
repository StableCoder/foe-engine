# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_simulation_imgui)

# Declaration
add_library(foe_simulation_imgui)
add_library(foe::simulation::imgui ALIAS foe_simulation_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_simulation_imgui PUBLIC foe_imgui foe_simulation)

target_code_coverage(foe_simulation_imgui)

if(MSVC)
  target_compile_options(foe_simulation_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_simulation_imgui PRIVATE -fno-exceptions)
endif()

# install
set_property(TARGET foe_simulation_imgui PROPERTY EXPORT_NAME simulation::imgui)

install(
  TARGETS foe_simulation_imgui
  EXPORT foe_simulation_imgui
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_simulation_imgui
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_simulation_imgui-config.cmake DESTINATION share/cmake/foe)
