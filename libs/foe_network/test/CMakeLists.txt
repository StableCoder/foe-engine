# Copyright (C) 2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_network)
add_test(NAME FoE-Network-Test COMMAND test_foe_network)

# Definition
target_sources(test_foe_network PRIVATE address.cpp c_header_compatibility.c
                                        result.cpp socket.cpp)

target_link_libraries(test_foe_network PRIVATE Catch2::Catch2WithMain
                                               foe_network)

target_code_coverage(
  test_foe_network
  AUTO
  ALL
  OBJECTS
  foe_network
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  ${BULLET_INCLUDE_DIRS})
