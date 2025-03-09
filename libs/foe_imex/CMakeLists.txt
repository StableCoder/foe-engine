# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imex)

# Declaration
add_library(foe_imex SHARED)
set_property(TARGET foe_imex PROPERTY EXPORT_NAME imex)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_imex PUBLIC foe_ecs foe_resource)

target_code_coverage(foe_imex)

if(MSVC)
  target_compile_options(foe_imex PRIVATE /EHsc)
else()
  target_compile_options(foe_imex PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/yaml)

if(BUILD_TESTS)
  add_subdirectory(test)
endif()
