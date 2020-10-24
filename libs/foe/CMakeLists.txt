cmake_minimum_required(VERSION 3.13)
project(foe)

# Dependencies

# Declaration
add_library(foe)

# Definition
generate_export_header(foe EXPORT_MACRO_NAME FOE_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/export.h)

target_include_directories(foe PUBLIC include
                                      ${CMAKE_CURRENT_BINARY_DIR}/exports)
# target_link_libraries(foe PUBLIC)
target_code_coverage(foe)

add_subdirectory(src)
