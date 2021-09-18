cmake_minimum_required(VERSION 3.15)
project(foe_wsi_imgui)

# Declaration
add_library(foe_wsi_imgui)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_wsi_imgui EXPORT_MACRO_NAME FOE_WSI_IMGUI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/wsi/imgui/export.h)

target_include_directories(foe_wsi_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_wsi_imgui PUBLIC foe_imgui foe_wsi)

target_code_coverage(foe_wsi_imgui)

if(MSVC)
  target_compile_options(foe_wsi_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_wsi_imgui PRIVATE -fno-exceptions)
endif()

# Special This extra export for the regular foe_wsi library is because it is an
# interface library which does not have a generated export header itself
generate_export_header(
  foe_wsi_imgui EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/wsi/export.h)
