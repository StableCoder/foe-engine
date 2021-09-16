cmake_minimum_required(VERSION 3.15)
project(foe_graphics_resource)

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
add_library(foe_graphics_resource)

# Definition
generate_export_header(
  foe_graphics_resource EXPORT_MACRO_NAME FOE_GFX_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/graphics/resource/export.h)

target_include_directories(
  foe_graphics_resource PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports
                               ${freeimage_INCLUDE_DIRS})

target_link_libraries(
  foe_graphics_resource
  PUBLIC foe
         foe_ecs
         foe_graphics
         foe_graphics_vk
         foe_model
         foe_model_assimp
         foe_resource
         foe_simulation
         vk_struct_cleanup
         ${freeimage_LIBRARIES})

target_code_coverage(foe_graphics_resource)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs)
