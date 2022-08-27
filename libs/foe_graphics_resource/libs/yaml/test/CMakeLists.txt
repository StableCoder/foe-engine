# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_graphics_resource_yaml)
add_test(NAME FoE-Graphics-Resource-Yaml-Test
         COMMAND test_foe_graphics_resource_yaml)

# Definition
target_sources(test_foe_graphics_resource_yaml PRIVATE result.cpp)

target_link_libraries(test_foe_graphics_resource_yaml
                      PRIVATE catch foe_graphics_resource_yaml)

target_code_coverage(
  test_foe_graphics_resource_yaml
  AUTO
  ALL
  OBJECTS
  foe_graphics_resource_yaml
  EXCLUDE
  .*/test/.*)
