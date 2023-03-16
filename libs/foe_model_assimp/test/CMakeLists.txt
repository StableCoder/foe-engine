# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Test Setup
enable_testing()
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-Model-Assimp-Tests COMMAND test_foe_model_assimp)
add_executable(test_foe_model_assimp)

# Definition
target_sources(test_foe_model_assimp PRIVATE flags.cpp)

target_link_libraries(test_foe_model_assimp PRIVATE foe_model_assimp
                                                    Catch2::Catch2WithMain)

target_code_coverage(
  test_foe_model_assimp
  AUTO
  ALL
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  OBJECTS
  foe_model_assimp)
