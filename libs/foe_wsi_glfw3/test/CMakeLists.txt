# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

# Setup
prepare_catch(COMPILED_CATCH)

# Declaration
add_executable(test_foe_wsi_glfw3)
add_test(NAME FoE-WSI-GLFW3-Test COMMAND test_foe_wsi_glfw3)

# Definition
target_sources(test_foe_wsi_glfw3 PRIVATE result.cpp)

if(BUILD_WINDOW_TESTS)
  target_sources(test_foe_wsi_glfw3 PRIVATE window.cpp)
endif()
if(BUILD_VULKAN_RUNTIME_TESTS)
  target_sources(test_foe_wsi_glfw3 PRIVATE vulkan.cpp)
endif()

target_include_directories(test_foe_wsi_glfw3
                           PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../private)

target_link_libraries(test_foe_wsi_glfw3 PRIVATE catch foe_wsi_glfw3)
target_code_coverage(
  test_foe_wsi_glfw3
  AUTO
  ALL
  OBJECTS
  foe_wsi_glfw3
  EXCLUDE
  ${CMAKE_CURRENT_SOURCE_DIR}/.*
  ${CMAKE_SOURCE_DIR}/external/.*)
