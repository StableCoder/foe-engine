# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_ecs_yaml CXX)

# Declaration
add_library(foe_ecs_yaml SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_ecs_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/ecs/yaml/export.h)

target_include_directories(foe_ecs_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_ecs_yaml PUBLIC foe_ecs foe_yaml)

target_code_coverage(foe_ecs_yaml)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
