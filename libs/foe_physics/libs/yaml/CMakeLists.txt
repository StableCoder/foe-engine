cmake_minimum_required(VERSION 3.13)
project(foe_physics_yaml)

# Declaration
add_library(foe_physics_yaml)

# Definition
generate_export_header(
  foe_physics_yaml EXPORT_MACRO_NAME FOE_PHYSICS_YAML_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/physics/yaml/export.h)

target_include_directories(foe_physics_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_physics_yaml PUBLIC foe_ecs_yaml foe_physics foe_yaml
                                              foe_imex_yaml)

if(UNIX)
  target_compile_options(foe_physics_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(foe_physics_yaml PRIVATE /EHa)
endif()

target_code_coverage(foe_physics_yaml)

add_subdirectory(src)
