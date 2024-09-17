# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Test Setup
enable_testing()
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-ECS-Yaml-Tests COMMAND test_foe_ecs_yaml)
add_executable(test_foe_ecs_yaml)

# Definition
target_sources(test_foe_ecs_yaml PRIVATE id.cpp indexes.cpp)

target_link_libraries(test_foe_ecs_yaml PRIVATE foe_ecs_yaml
                                                Catch2::Catch2WithMain)

if(MSVC)
target_compile_options(test_foe_ecs_yaml PRIVATE /EHa)
else()
  target_compile_options(test_foe_ecs_yaml PRIVATE -fexceptions)
endif()

target_code_coverage(
  test_foe_ecs_yaml
  AUTO
  ALL
  EXCLUDE
  /usr/*
  .*.test..*
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*
  OBJECTS
  foe_ecs_yaml)
