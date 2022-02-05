cmake_minimum_required(VERSION 3.15)
project(foe_imgui_vk)

# Declaration
add_library(foe_imgui_vk)

# Definition
add_subdirectory(src)

generate_export_header(foe_imgui_vk EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/imgui/vk/export.h)

target_include_directories(foe_imgui_vk
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_imgui_vk PUBLIC foe_graphics_vk foe_imgui)

target_code_coverage(foe_imgui_vk)
