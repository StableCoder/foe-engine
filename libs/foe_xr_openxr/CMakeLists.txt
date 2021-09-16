cmake_minimum_required(VERSION 3.15)
project(foe_xr_openxr)

# Dependencies
find_package(OpenXR REQUIRED)
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_xr_openxr)

# Definition
generate_export_header(
  foe_xr_openxr EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/xr/export.h)

target_include_directories(foe_xr_openxr
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_compile_definitions(foe_xr_openxr PUBLIC FOE_XR_SUPPORT)

target_link_libraries(foe_xr_openxr PUBLIC foe_xr openxr_loader Vulkan::Vulkan)

target_code_coverage(foe_xr_openxr)

add_subdirectory(src)
