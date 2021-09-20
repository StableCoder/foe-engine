cmake_minimum_required(VERSION 3.15)
project(foe_wsi_imgui)

# Declaration
add_library(foe_wsi_imgui)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_wsi_imgui EXPORT_MACRO_NAME FOE_WSI_IMGUI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/wsi/imgui/export.h)

target_include_directories(foe_wsi_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_wsi_imgui PUBLIC foe_imgui)

target_code_coverage(foe_wsi_imgui)

if(MSVC)
  target_compile_options(foe_wsi_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_wsi_imgui PRIVATE -fno-exceptions)
endif()

if(WSI_LOADER)
  target_link_libraries(foe_wsi_imgui PUBLIC foe_wsi_loader)
else()
  target_link_libraries(foe_wsi_imgui PUBLIC foe_wsi_${WSI_LIBRARY})
endif()
