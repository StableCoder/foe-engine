# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_graphics)

# Declaration
add_library(foe_graphics SHARED)
add_library(foe::graphics ALIAS foe_graphics)

# Definition
generate_export_header(
  foe_graphics EXPORT_MACRO_NAME FOE_GFX_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h)

target_link_libraries(foe_graphics PUBLIC foe_core)

target_code_coverage(foe_graphics)

if(MSVC)
  target_compile_options(foe_graphics PRIVATE /EHsc)
else()
  target_compile_options(foe_graphics PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_graphics
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/graphics/export.h
         include/foe/graphics/backend.h
         include/foe/graphics/builtin_descriptor_sets.h
         include/foe/graphics/delayed_caller.h
         include/foe/graphics/mesh.h
         include/foe/graphics/render_target.h
         include/foe/graphics/render_view_pool.h
         include/foe/graphics/result.h
         include/foe/graphics/runtime.h
         include/foe/graphics/session.h
         include/foe/graphics/shader.h
         include/foe/graphics/type_defs.h
         include/foe/graphics/upload_buffer.h
         include/foe/graphics/upload_context.h
         include/foe/graphics/upload_request.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_graphics PROPERTY EXPORT_NAME foe::graphics)

install(
  TARGETS foe_graphics
  EXPORT foe_graphics
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(EXPORT foe_graphics DESTINATION share/cmake/foe)

install(FILES foe_graphics-config.cmake DESTINATION share/cmake/foe)
