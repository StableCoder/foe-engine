cmake_minimum_required(VERSION 3.13)
project(foe_resource)

# Dependencies
find_file(freeimage_INCLUDE_DIRS FreeImage.h)
find_library(freeimage_LIBRARIES freeimage)
if(freeimage_INCLUDE_DIRS-NOTFOUND OR freeimage_LIBRARIES-NOTFOUND)
  message(FATAL_ERROR "Cannot find required FreeImage library!")
else()
  get_filename_component(freeimage_INCLUDE_DIRS ${freeimage_INCLUDE_DIRS}
                         DIRECTORY)
endif()
mark_as_advanced(FORCE freeimage_INCLUDE_DIRS freeimage_LIBRARIES)

# Declaration
add_library(foe_resource)

# Definition
generate_export_header(
  foe_resource EXPORT_MACRO_NAME FOE_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/resource/export.h)

target_compile_definitions(
  foe_resource
  PUBLIC ASSIMP_PLUGIN_PATH="$<TARGET_FILE:foe_model_importer_assimp>")
target_include_directories(
  foe_resource PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports
                      ${freeimage_INCLUDE_DIRS})
target_link_libraries(foe_resource PUBLIC foe foe_graphics foe_graphics_vk
                                          foe_model ${freeimage_LIBRARIES})
target_code_coverage(foe_resource)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs)
