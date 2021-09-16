cmake_minimum_required(VERSION 3.15)
project(foe_physics)

# Dependencies
find_package(Bullet REQUIRED)

# Declaration
add_library(foe_physics)

# Definition
generate_export_header(foe_physics EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/physics/export.h)

target_include_directories(
  foe_physics PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports
                     ${BULLET_INCLUDE_DIRS})

target_link_libraries(foe_physics PUBLIC foe foe_ecs foe_imex foe_position
                                         foe_resource ${BULLET_LIBRARIES})

target_code_coverage(foe_physics)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)
