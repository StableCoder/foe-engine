# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
find_package(Catch2 3 REQUIRED)

# Declaration
add_test(NAME FoE-Graphics-Yaml-Test COMMAND test_foe_graphics_vk_yaml)
add_executable(test_foe_graphics_vk_yaml)

# Definition
target_sources(
  test_foe_graphics_vk_yaml
  PRIVATE vk_enums.cpp vk_struct_parsing.cpp
          vk_struct_VkPipelineDepthStencilStateCreateInfo.cpp
          vk_struct_VkStencilOpState.cpp)

target_compile_definitions(
  test_foe_graphics_vk_yaml
  PRIVATE
    FUZZED_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/fuzzed_test_data")

target_link_libraries(
  test_foe_graphics_vk_yaml PRIVATE Catch2::Catch2WithMain foe_graphics_vk_yaml
                                    vk_struct_compare)

if(UNIX
   OR CMAKE_C_COMPILER_ID MATCHES "[Cc]lang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang")
  target_compile_options(test_foe_graphics_vk_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(test_foe_graphics_vk_yaml PRIVATE /EHa)
endif()

target_code_coverage(
  test_foe_graphics_vk_yaml
  AUTO
  ALL
  OBJECTS
  foe_graphics_vk_yaml
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)

# Auxiliary Targets
if(AFL)
  add_subdirectory(fuzz)
endif()
