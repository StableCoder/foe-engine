# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_resource)
add_test(NAME FoE-Resource-Test COMMAND test_foe_resource)

# Definition
target_sources(
  test_foe_resource
  PRIVATE c_header_compatibility.c
          create_info.cpp
          load_state.cpp
          pool.cpp
          resource.cpp
          resource_loading.cpp
          resource_replacement.cpp
          resource_unloading.cpp
          result.cpp)

target_link_libraries(test_foe_resource PRIVATE catch foe_resource)

target_code_coverage(
  test_foe_resource
  AUTO
  ALL
  OBJECTS
  foe_resource
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
