cmake_minimum_required(VERSION 3.13)
project(foe_graphics)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_graphics)

# Definition
generate_export_header(
  foe_graphics EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/export.h)

target_include_directories(foe_graphics
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_graphics PUBLIC Vulkan::Vulkan foe)
target_code_coverage(foe_graphics)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/yaml)
