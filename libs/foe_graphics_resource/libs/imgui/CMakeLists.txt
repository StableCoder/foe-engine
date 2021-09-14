cmake_minimum_required(VERSION 3.13)
project(foe_graphics_resource_imgui)

# Declaration
add_library(foe_graphics_resource_imgui)

# Definition
generate_export_header(
  foe_graphics_resource_imgui EXPORT_MACRO_NAME FOE_GFX_RES_IMGUI_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/resource/imgui/export.h)

target_include_directories(foe_graphics_resource_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(
  foe_graphics_resource_imgui PUBLIC foe_graphics_resource foe_imgui
                                     foe_simulation_imgui)

target_code_coverage(foe_graphics_resource_imgui)

add_subdirectory(src)
