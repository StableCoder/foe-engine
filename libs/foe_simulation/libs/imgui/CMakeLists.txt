cmake_minimum_required(VERSION 3.13)
project(foe_simulation_imgui)

# Declaration
add_library(foe_simulation_imgui)

# Definition
generate_export_header(
  foe_simulation_imgui EXPORT_MACRO_NAME FOE_SIM_IMGUI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/simulation/imgui/export.h)

target_include_directories(foe_simulation_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_simulation_imgui PUBLIC foe_imgui foe_simulation)

target_code_coverage(foe_simulation_imgui)

add_subdirectory(src)
