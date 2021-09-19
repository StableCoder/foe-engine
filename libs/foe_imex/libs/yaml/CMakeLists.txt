cmake_minimum_required(VERSION 3.15)
project(foe_imex_yaml)

# Declaration
add_library(foe_imex_yaml)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_imex_yaml EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/imex/yaml/export.h)

target_include_directories(foe_imex_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_imex_yaml PUBLIC foe_imex foe_yaml foe_ecs_yaml
                                           foe_resource)

target_code_coverage(foe_imex_yaml)

# Auxiliary Targets
