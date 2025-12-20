# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_xr)

# Declaration
add_library(foe_xr INTERFACE)
add_library(foe::xr ALIAS foe_xr)

# Definition
set_target_properties(foe_xr PROPERTIES DEBUG_POSTFIX "_d")

if(XR_SUPPORT)
  target_compile_definitions(foe_xr INTERFACE FOE_XR_SUPPORT)
endif()

target_link_libraries(foe_xr INTERFACE foe_core)

target_sources(
  foe_xr
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         include/foe/xr/runtime.h
         include/foe/xr/session.h)

# install
set_property(TARGET foe_xr PROPERTY EXPORT_NAME foe::xr)

install(
  TARGETS foe_xr
  EXPORT foe_xr
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_xr DESTINATION share/cmake/foe)

install(FILES foe_xr-config.cmake DESTINATION share/cmake/foe)
