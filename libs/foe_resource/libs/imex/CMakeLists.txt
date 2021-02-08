cmake_minimum_required(VERSION 3.13)
project(foe_resource_imex)

# Declaration
add_library(foe_resource_imex INTERFACE)

# Definition
generate_export_header(
  foe_resource_imex EXPORT_MACRO_NAME FOE_RES_IMEX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/resource/imex/export.h)

target_include_directories(
  foe_resource_imex INTERFACE include ${CMAKE_CURRENT_BINARY_DIR}/exports)
