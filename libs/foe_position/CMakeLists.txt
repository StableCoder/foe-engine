cmake_minimum_required(VERSION 3.15)
project(foe_position)

# Dependencies
find_package(glm REQUIRED)

# Declaration
add_library(foe_position)

# Definition
generate_export_header(
  foe_position EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/position/export.h)

target_include_directories(foe_position
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_position PUBLIC foe foe_imex foe_simulation glm)

target_code_coverage(foe_position PUBLIC)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)
