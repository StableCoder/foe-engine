cmake_minimum_required(VERSION 3.13)
project(foe_imex)

# Declaration
add_library(foe_imex)

# Definition
generate_export_header(foe_imex EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/imex/export.h)

target_include_directories(foe_imex PUBLIC include
                                           ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_imex PUBLIC foe_ecs)
target_code_coverage(foe_imex)

add_subdirectory(src)

# Auxiliary Targets
