cmake_minimum_required(VERSION 3.15)
project(foe_graphics_yaml)

# Declaration
add_library(foe_graphics_yaml)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_graphics_yaml EXPORT_MACRO_NAME FOE_GFX_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/graphics/yaml/export.h)

target_include_directories(
  foe_graphics_yaml PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public ../../src)

target_link_libraries(foe_graphics_yaml PUBLIC foe_graphics foe_yaml)

target_code_coverage(foe_graphics_yaml)

# Auxiliary Targets
