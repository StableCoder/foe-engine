# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-ECS-Test COMMAND test_foe_ecs)
add_executable(test_foe_ecs)

# Definition
target_sources(
  test_foe_ecs
  PRIVATE c_header_compatibility.c
          component_pool_entity_list.cpp
          component_pool.cpp
          editor_name_map.cpp
          entity_list.cpp
          group_translator.cpp
          id.cpp
          indexes.cpp
          result.cpp)

target_link_libraries(test_foe_ecs PRIVATE catch foe_ecs)

target_code_coverage(
  test_foe_ecs
  AUTO
  ALL
  OBJECTS
  foe_ecs
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
