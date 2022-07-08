# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Test Setup
enable_testing()
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Model-Assimp-Tests COMMAND test_foe_model_assimp)
add_executable(test_foe_model_assimp)

# Definition
target_sources(test_foe_model_assimp PRIVATE flags.cpp)

target_link_libraries(test_foe_model_assimp PRIVATE foe_model_assimp catch)

target_code_coverage(
  test_foe_model_assimp
  AUTO
  ALL
  EXCLUDE
  .*/external/.*
  .*/test/.*
  OBJECTS
  foe_model_assimp)
