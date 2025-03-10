# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_wsi)

# Options
option(BUILD_WINDOW_TESTS
       "Enables building tests that utilize Window subsystems" OFF)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_wsi INTERFACE)

# Definition
add_subdirectory(include)

target_link_libraries(foe_wsi INTERFACE foe_core Vulkan::Vulkan)

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/loader)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_wsi PROPERTY EXPORT_NAME wsi)

install(
  TARGETS foe_wsi
  EXPORT foe_wsi
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_wsi
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_wsi-config.cmake DESTINATION share/cmake/foe)
