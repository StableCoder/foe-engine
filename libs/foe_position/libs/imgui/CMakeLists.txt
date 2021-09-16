cmake_minimum_required(VERSION 3.15)
project(foe_position_imgui)

# Declaration
add_library(foe_position_imgui)

# Definition
generate_export_header(
  foe_position_imgui EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/position/imgui/export.h)

target_include_directories(foe_position_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_position_imgui PUBLIC foe_imgui foe_position
                                                foe_simulation_imgui)

target_code_coverage(foe_position_imgui)

add_subdirectory(src)
