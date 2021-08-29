cmake_minimum_required(VERSION 3.13)
project(foe_xr)

# Declaration
add_library(foe_xr)

# Definition
generate_export_header(foe_xr EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/xr/export.h)

target_include_directories(foe_xr PUBLIC include
                                         ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_xr PUBLIC foe)

add_subdirectory(src)
