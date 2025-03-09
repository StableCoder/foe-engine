# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
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
add_library(foe_graphics_resource SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_include_directories(foe_graphics_resource
                           PRIVATE ${freeimage_INCLUDE_DIRS})

target_link_libraries(
  foe_graphics_resource
  PUBLIC foe_core
         foe_ecs
         foe_graphics
         foe_graphics_vk
         foe_model
         foe_model_assimp
         foe_simulation
         vk_struct_cleanup
         ${freeimage_LIBRARIES})

target_code_coverage(foe_graphics_resource)

if(MSVC)
  target_compile_options(foe_graphics_resource PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics_resource PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
