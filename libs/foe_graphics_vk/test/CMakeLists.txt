# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_test(NAME FoE-Graphics-VK-Test COMMAND test_foe_graphics_vk)
add_executable(test_foe_graphics_vk)

# Definition
target_sources(test_foe_graphics_vk PRIVATE c_header_compatibility.c result.cpp
                                            sample_count.cpp)

if(BUILD_VULKAN_RUNTIME_TESTS)
  target_sources(test_foe_graphics_vk PRIVATE runtime.cpp session.cpp
                                              shader.cpp)
endif()

target_link_libraries(test_foe_graphics_vk PRIVATE catch foe_graphics_vk)

target_code_coverage(
  test_foe_graphics_vk
  AUTO
  ALL
  OBJECTS
  foe_graphics_vk
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
