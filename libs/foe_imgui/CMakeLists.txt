cmake_minimum_required(VERSION 3.13)
project(foe_imgui)

# Declaration
add_library(foe_imgui)

# Definition
generate_export_header(foe_imgui EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/imgui_export.h)

target_include_directories(foe_imgui PUBLIC include
                                            ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_imgui PUBLIC foe_graphics imgui)

add_subdirectory(src)
