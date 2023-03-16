# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_executable(test_foe_imex_yaml)
add_test(NAME FoE-ImEx-Yaml-Test COMMAND test_foe_imex_yaml)

# Definition
target_sources(
  test_foe_imex_yaml
  PRIVATE c_header_compatibility.c create_importer.cpp imex_registration.cpp
          importer.cpp result.cpp test_importer.cpp)

target_compile_definitions(
  test_foe_imex_yaml PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

target_link_libraries(test_foe_imex_yaml PRIVATE Catch2::Catch2WithMain
                                                 foe_imex_yaml)

target_code_coverage(
  test_foe_imex_yaml
  AUTO
  ALL
  OBJECTS
  foe_imex_yaml
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
