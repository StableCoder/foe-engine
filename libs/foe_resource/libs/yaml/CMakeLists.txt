cmake_minimum_required(VERSION 3.15)
project(foe_resource_yaml)

# Declaration
add_library(foe_resource_yaml)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_resource_yaml EXPORT_MACRO_NAME FOE_RES_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/resource/yaml/export.h)

target_include_directories(foe_resource_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(
  foe_resource_yaml PUBLIC foe_ecs_yaml foe_graphics_vk_yaml foe_imex_yaml
                           foe_resource)

target_code_coverage(foe_resource_yaml)
