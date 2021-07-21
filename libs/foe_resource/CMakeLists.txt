cmake_minimum_required(VERSION 3.13)
project(foe_resource)

# Declaration
add_library(foe_resource)

# Definition
generate_export_header(
  foe_resource EXPORT_MACRO_NAME FOE_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/resource/export.h)

target_compile_definitions(
  foe_resource
  PUBLIC ASSIMP_PLUGIN_PATH="$<TARGET_FILE:foe_model_importer_assimp>")
target_include_directories(
  foe_resource PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(
  foe_resource
  PUBLIC foe
         foe_ecs
         foe_model
         foe_simulation)
target_code_coverage(foe_resource)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs)
