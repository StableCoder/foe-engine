cmake_minimum_required(VERSION 3.15)
project(foe_graphics_vk)

# Declaration
add_library(foe_graphics_vk SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_graphics_vk EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/graphics/export.h)

target_include_directories(foe_graphics_vk
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(
  foe_graphics_vk PUBLIC foe_graphics vulkan_memory_allocator vk_struct_cleanup
                         vk_value_serialization)
target_link_libraries(foe_graphics_vk PRIVATE khronos_mini_libs)

target_code_coverage(foe_graphics_vk)

if(APPLE)
  target_compile_definitions(foe_graphics_vk PRIVATE VK_USE_PLATFORM_MACOS_MVK)
endif()

if(MSVC)
  target_compile_options(foe_graphics_vk PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_vk PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
