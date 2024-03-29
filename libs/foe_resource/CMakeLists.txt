# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_resource)

# Declaration
add_library(foe_resource SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_resource EXPORT_MACRO_NAME FOE_RES_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/resource/export.h)

target_include_directories(foe_resource
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_resource PUBLIC foe foe_ecs)

target_code_coverage(foe_resource)

if(MSVC)
  target_compile_options(foe_resource PRIVATE /EHsc)
else()
  target_compile_options(foe_resource PRIVATE -fno-exceptions)
endif()

# Other
add_subdirectory(libs/imgui)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
