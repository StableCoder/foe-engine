cmake_minimum_required(VERSION 3.15)
project(foe_graphics_resource_imgui)

# Declaration
add_library(foe_graphics_resource_imgui)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_graphics_resource_imgui EXPORT_MACRO_NAME FOE_GFX_RES_IMGUI_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/graphics/resource/imgui/export.h)

target_include_directories(foe_graphics_resource_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(
  foe_graphics_resource_imgui PUBLIC foe_graphics_resource foe_imgui
                                     foe_simulation_imgui)

target_code_coverage(foe_graphics_resource_imgui)

if(MSVC)
  target_compile_options(foe_graphics_resource_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_resource_imgui PRIVATE -fno-exceptions)
endif()
