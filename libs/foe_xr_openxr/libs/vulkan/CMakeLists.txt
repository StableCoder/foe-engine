cmake_minimum_required(VERSION 3.15)
project(foe_xr_openxr_vk)

# Declaration
add_library(foe_xr_openxr_vk)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_xr_openxr_vk EXPORT_MACRO_NAME FOE_OPENXR_VK_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/xr/openxr/vk/export.h)

target_include_directories(foe_xr_openxr_vk
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_xr_openxr_vk PUBLIC foe_xr_openxr foe_graphics_vk)

target_code_coverage(foe_xr_openxr_vk)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
