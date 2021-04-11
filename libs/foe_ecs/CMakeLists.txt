cmake_minimum_required(VERSION 3.13)
project(foe_ecs)

# Declaration
add_library(foe_ecs)

# Definition
generate_export_header(foe_ecs EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/ecs/export.h)

target_include_directories(foe_ecs PUBLIC include
                                          ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_ecs PUBLIC foe)
target_code_coverage(foe_ecs)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/yaml)

if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
  add_subdirectory(test)
endif()
