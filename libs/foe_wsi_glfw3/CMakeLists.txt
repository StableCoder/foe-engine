cmake_minimum_required(VERSION 3.15)
project(foe_wsi_glfw3)

# To use this library as the default WSI library, use set WSI_LIBRARY to 'glfw3'
# in the CMake project settings

# Dependencies
find_package(glfw3 REQUIRED)

# Declaration
if(WSI_LOADER)
  # If not directly using this library, eg. through a loader, make sure it is
  # built as a shared object
  add_library(foe_wsi_glfw3 SHARED)
else()
  add_library(foe_wsi_glfw3)
endif()

# Definition
add_subdirectory(src)

target_include_directories(foe_wsi_glfw3 PUBLIC ${glfw_INCLUDE_DIR})

target_link_libraries(foe_wsi_glfw3 PUBLIC foe_wsi glfw)
target_link_libraries(foe_wsi_glfw3 PRIVATE khronos_mini_libs)

target_code_coverage(foe_wsi_glfw3)

if(MSVC)
  target_compile_options(foe_wsi_glfw3 PRIVATE /EHsc)
else()
  target_compile_options(foe_wsi_glfw3 PRIVATE -fno-exceptions)
endif()

if(NOT WSI_LOADER AND WSI_LIBRARY STREQUAL "glfw3")
  # Export the WSI_EXPORT header from this library
  generate_export_header(
    foe_wsi_glfw3 EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/public/foe/wsi/export.h)

  target_include_directories(foe_wsi_glfw3
                             PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/public)
else()
  # Don't export the WSI_EXPORT header from this library, use local use only
  generate_export_header(
    foe_wsi_glfw3 EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/private/foe/wsi/export.h)

  target_include_directories(foe_wsi_glfw3
                             PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/private)
endif()

# Auxiliary Targets
if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
