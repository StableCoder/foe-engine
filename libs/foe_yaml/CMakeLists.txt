# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_yaml)

# Dependencies
find_package(glm REQUIRED)
find_package(yaml-cpp REQUIRED)

# Declaration
add_library(foe_yaml SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/yaml/export.h)

target_include_directories(
  foe_yaml PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public
                  ${GLM_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS}/include)

if(WIN32 OR APPLE)
  target_link_libraries(foe_yaml PUBLIC yaml-cpp::yaml-cpp)
else()
  target_link_libraries(foe_yaml PUBLIC yaml-cpp)
endif()

target_code_coverage(foe_yaml)

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
