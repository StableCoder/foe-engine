# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_xr)

# Declaration
add_library(foe_xr INTERFACE)
add_library(foe::xr ALIAS foe_xr)

# Definition
add_subdirectory(include)

if(XR_SUPPORT)
  target_compile_definitions(foe_xr INTERFACE FOE_XR_SUPPORT)
endif()

target_link_libraries(foe_xr INTERFACE foe_core)

# install
set_property(TARGET foe_xr PROPERTY EXPORT_NAME xr)

install(
  TARGETS foe_xr
  EXPORT foe_xr
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_xr
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_xr-config.cmake DESTINATION share/cmake/foe)
