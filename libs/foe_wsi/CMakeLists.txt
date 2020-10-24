cmake_minimum_required(VERSION 3.13)
project(foe_wsi)

# Dependencies
find_path(glfw_INCLUDE_DIR NAMES GLFW/glfw3.h)
mark_as_advanced(glfw_INCLUDE_DIR)
find_library(GLFW NAMES glfw glfw3 glfw3dll)
mark_as_advanced(GLFW)

# Declaration
add_library(foe_wsi)

# Definition
generate_export_header(foe_wsi EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/wsi_export.h)

target_include_directories(foe_wsi PUBLIC include ${glfw_INCLUDE_DIR}
                                          ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_wsi PUBLIC ${GLFW})
target_code_coverage(foe_wsi)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
