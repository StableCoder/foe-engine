cmake_minimum_required(VERSION 3.13)
project(foe_wsi_glfw3)

# Dependencies
find_package(glfw3 REQUIRED)

# Declaration
add_library(foe_wsi_glfw3)

# Definition
generate_export_header(
  foe_wsi_glfw3 EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/wsi/export.h)

target_include_directories(
  foe_wsi_glfw3 PUBLIC include ${glfw_INCLUDE_DIR}
                       ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_wsi_glfw3 PUBLIC foe_wsi glfw)

target_code_coverage(foe_wsi_glfw3)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
