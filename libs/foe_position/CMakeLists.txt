# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position)

# Dependencies
find_package(glm REQUIRED)

# Declaration
add_library(foe_position SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_position PUBLIC foe_core foe_imex foe_simulation)

target_code_coverage(foe_position PUBLIC)

if(DEFINED GLM_INCLUDE_DIRS)
  target_include_directories(foe_position PUBLIC ${GLM_INCLUDE_DIRS}
                                                 ${GLM_INCLUDE_DIRS}/include)
elseif(TARGET glm::glm)
  target_link_libraries(foe_position PUBLIC glm::glm)
else()
  target_link_libraries(foe_position PUBLIC glm::glm-header-only)
endif()

if(MSVC)
  target_compile_options(foe_position PRIVATE /EHsc)
else()
  target_compile_options(foe_position PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)
