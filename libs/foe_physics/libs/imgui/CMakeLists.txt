cmake_minimum_required(VERSION 3.13)
project(foe_physics_imgui)

# Declaration
add_library(foe_physics_imgui)

# Definition
generate_export_header(
  foe_physics_imgui EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/physics/imgui/export.h)

target_include_directories(foe_physics_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_physics_imgui PUBLIC foe_imgui foe_physics
                                               foe_simulation_imgui)

target_code_coverage(foe_physics_imgui)

add_subdirectory(src)
