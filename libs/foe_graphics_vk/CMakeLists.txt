cmake_minimum_required(VERSION 3.13)
project(foe_graphics_vk)

# Declaration
add_library(foe_graphics_vk)

# Definition
generate_export_header(
  foe_graphics_vk EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/export.h)

target_include_directories(foe_graphics_vk
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_graphics_vk
                      PUBLIC foe_graphics vulkan_memory_allocator vk_error_code)

target_code_coverage(foe_graphics_vk)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
