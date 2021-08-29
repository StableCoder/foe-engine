cmake_minimum_required(VERSION 3.13)
project(foe_graphics_vk_yaml)

# Declaration
add_library(foe_graphics_vk_yaml)

# Definition
generate_export_header(
  foe_graphics_vk_yaml EXPORT_MACRO_NAME FOE_GFX_VK_YAML_EXPORT
  EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/vk/yaml/export.h)

target_include_directories(
  foe_graphics_vk_yaml PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports
                              ../../src)
target_link_libraries(
  foe_graphics_vk_yaml PUBLIC foe_graphics_yaml foe_graphics_vk foe_yaml
                              vk_struct_cleanup vk_value_serialization)

if(UNIX)
  target_compile_options(foe_graphics_vk_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(foe_graphics_vk_yaml PRIVATE /EHa)
endif()

target_code_coverage(foe_graphics_vk_yaml)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
