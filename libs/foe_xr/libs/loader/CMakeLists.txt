cmake_minimum_required(VERSION 3.15)
project(foe_xr_loader)

# Declaration
add_library(foe_xr_loader)

# Definition
add_subdirectory(src)

target_include_directories(foe_xr_loader PUBLIC include)

target_link_libraries(foe_xr_loader PUBLIC foe_xr)

target_code_coverage(foe_xr_loader)

if(MSVC)
  target_compile_options(foe_xr_loader PRIVATE /EHsc)
else()
  target_compile_options(foe_xr_loader PRIVATE -fno-exceptions)
endif()

if(XR_SUPPORT AND XR_LOADER)
  # If set to use the XR 'loader', then define an inherited flag that indicates
  # for any downstream projects to add the code for loading XR implementations
  # dynamically (via foeWsiLoadImplementation) and where the default
  # implementation to load is (if implemented)
  target_compile_definitions(
    foe_xr_loader
    PUBLIC XR_LOADER
           DEFAULT_XR_IMPLEMENTATION="$<TARGET_FILE:foe_xr_${XR_LIBRARY}>")
endif()

if(NOT XR_SUPPORT OR XR_LOADER)
  # If XR support isn't enabled, still need to export the header from something,
  # might as well be the common loader

  # Export the XR_EXPORT header from this library.
  generate_export_header(
    foe_xr_loader EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/public/foe/xr/export.h)

  target_include_directories(foe_xr_loader
                             PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public)
else()
  # Don't export the WSI_EXPORT header from this library, local use only
  generate_export_header(
    foe_xr_loader EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/private/foe/xr/export.h)

  target_include_directories(foe_xr_loader
                             PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/private)
endif()
