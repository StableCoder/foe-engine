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
add_library(foe::yaml ALIAS foe_yaml)

# Definition
generate_export_header(foe_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/yaml/export.h)

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

target_sources(
  foe_yaml
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/yaml/export.h
         include/foe/yaml/exception.hpp
         include/foe/yaml/glm_colour_parsing.hpp
         include/foe/yaml/glm.hpp
         include/foe/yaml/pod.hpp)

add_subdirectory(src)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_yaml PROPERTY EXPORT_NAME yaml)

install(
  TARGETS foe_yaml
  EXPORT foe_yaml
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_yaml
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_yaml-config.cmake DESTINATION share/cmake/foe)
