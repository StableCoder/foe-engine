# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position_yaml)

# Declaration
add_library(foe_position_yaml SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_position_yaml PUBLIC foe_imex_yaml foe_position)

target_code_coverage(foe_position_yaml)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
