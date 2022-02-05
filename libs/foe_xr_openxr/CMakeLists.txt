cmake_minimum_required(VERSION 3.15)
project(foe_xr_openxr)

# To use this library as the default XR library, use set XR_LIBRARY to 'openxr'
# in the CMake project settings

# Dependencies
find_package(OpenXR REQUIRED)

# Declaration
if(XR_LOADER)
  # If not directly using this library, eg. through a loader, make sure it is
  # built as a shared object
  add_library(foe_xr_openxr SHARED)
else()
  add_library(foe_xr_openxr)
endif()

# Definition
generate_export_header(
  foe_xr_openxr EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/xr/export.h)

target_include_directories(foe_xr_openxr
                           PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public)

add_subdirectory(src)

target_include_directories(foe_xr_openxr PUBLIC include)

target_link_libraries(foe_xr_openxr PUBLIC foe_xr openxr_loader)

target_code_coverage(foe_xr_openxr)

if(MSVC)
  target_compile_options(foe_xr_openxr PRIVATE /EHsc)
else()
  target_compile_options(foe_xr_openxr PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/vulkan)
