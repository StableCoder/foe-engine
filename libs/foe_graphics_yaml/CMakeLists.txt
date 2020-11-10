cmake_minimum_required(VERSION 3.13)
project(foe_graphics)

# Declaration
add_library(foe_graphics_yaml)

# Definition
generate_export_header(
  foe_graphics_yaml EXPORT_MACRO_NAME FOE_GFX_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/yaml/export.h)

target_include_directories(foe_graphics_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_graphics_yaml PUBLIC foe foe_graphics foe_yaml
                                               vk_value_serialization)
target_code_coverage(foe_graphics_yaml)

add_subdirectory(src)

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
