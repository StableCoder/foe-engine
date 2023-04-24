# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_model_assimp)

# Dependencies
if(UNIX)
  # Assimp 5.0 CMake config files are broken on Linux, use the pkgconf ones
  find_package(PkgConfig)
  pkg_check_modules(assimp REQUIRED assimp)
else()
  find_package(assimp REQUIRED)
endif()

# Declaration
add_library(foe_model_assimp SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_model_assimp EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/model/assimp/export.h)

target_include_directories(
  foe_model_assimp PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                          ${assimp_INCLUDE_DIRS})

target_link_libraries(foe_model_assimp PUBLIC foe_model
                                              ${assimp_LINK_LIBRARIES})
if(NOT UNIX)
  target_link_libraries(foe_model_assimp PUBLIC assimp::assimp)
endif()

target_code_coverage(foe_model_assimp)

if(MSVC)
  target_compile_options(foe_model_assimp PRIVATE /EHsc)
else()
  target_compile_options(foe_model_assimp PRIVATE -fno-exceptions)
endif()

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
