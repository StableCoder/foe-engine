cmake_minimum_required(VERSION 3.13)
project(foe_graphics_resource_yaml)

# Declaration
add_library(foe_graphics_resource_yaml)

# Definition
generate_export_header(
  foe_graphics_resource_yaml EXPORT_MACRO_NAME FOE_GFX_RES_YAML_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/resource/yaml/export.h)

target_include_directories(foe_graphics_resource_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(
  foe_graphics_resource_yaml PUBLIC foe_ecs_yaml foe_graphics_resource
                                    foe_graphics_vk_yaml foe_imex_yaml)
target_code_coverage(foe_graphics_resource_yaml)

add_subdirectory(src)
