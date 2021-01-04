cmake_minimum_required(VERSION 3.13)
project(foe_xr)

# Dependencies
find_package(OpenXR REQUIRED)

# Declaration
add_library(foe_xr)

# Definition
generate_export_header(foe_xr EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/xr/export.h)

target_include_directories(foe_xr PUBLIC include
                                         ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_compile_definitions(foe_xr PUBLIC FOE_XR_SUPPORT)
target_link_libraries(foe_xr PUBLIC foe openxr_loader)
target_code_coverage(foe_xr)

add_subdirectory(src)
