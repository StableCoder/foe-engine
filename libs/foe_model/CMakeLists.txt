# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_model)

# Dependencies
find_package(glm REQUIRED)

# Declarations
add_library(foe_model)

# Library Definition
add_subdirectory(src)

generate_export_header(foe_model EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/model/export.h)

target_include_directories(
  foe_model PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                   ${GLM_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS}/include)

target_link_libraries(foe_model PUBLIC foe)

target_code_coverage(foe_model)

if(UNIX)
  target_compile_options(foe_model PRIVATE -fPIC)
endif()

if(MSVC)
  target_compile_options(foe_model PRIVATE /EHsc)
else()
  target_compile_options(foe_model PRIVATE -fno-exceptions)
endif()

# Auxiliary Target
