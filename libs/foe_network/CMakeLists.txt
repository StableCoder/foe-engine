# Copyright (C) 2023-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_network)

# Declaration
add_library(foe_network SHARED)
add_library(foe::network ALIAS foe_network)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_network PUBLIC foe_core)
if(WIN32)
  target_link_libraries(foe_network PUBLIC ws2_32)
endif()

target_code_coverage(foe_network)

if(MSVC)
  target_compile_options(foe_network PRIVATE /EHsc)
else()
  target_compile_options(foe_network PRIVATE -fno-exceptions)
endif()

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_network PROPERTY EXPORT_NAME network)

install(
  TARGETS foe_network
  EXPORT foe_network
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_network
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_network-config.cmake DESTINATION share/cmake/foe)
