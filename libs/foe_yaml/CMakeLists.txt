# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_yaml)

# Dependencies
find_package(glm REQUIRED)
find_package(yaml-cpp REQUIRED)

# Declaration
add_library(foe_yaml SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

if(DEFINED GLM_INCLUDE_DIRS)
  target_include_directories(foe_yaml PUBLIC ${GLM_INCLUDE_DIRS}
                                             ${GLM_INCLUDE_DIRS}/include)
elseif(TARGET glm::glm)
  target_link_libraries(foe_yaml PUBLIC glm::glm)
else()
  target_link_libraries(foe_yaml PUBLIC glm::glm-header-only)
endif()

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
