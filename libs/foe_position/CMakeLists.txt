# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_position)

# Dependencies
find_package(glm REQUIRED)

# Declaration
add_library(foe_position SHARED)
add_library(foe::position ALIAS foe_position)

# Definition
generate_export_header(foe_position EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/position/export.h)

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

target_sources(
  foe_position
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/position/export.h
         include/foe/position/component/3d.hpp
         include/foe/position/component/3d_pool.h
         include/foe/position/binary.h
         include/foe/position/compare.h
         include/foe/position/registration.h
         include/foe/position/result.h
         include/foe/position/type_defs.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/binary)
add_subdirectory(libs/imgui)
add_subdirectory(libs/yaml)

# install
set_property(TARGET foe_position PROPERTY EXPORT_NAME position)

install(
  TARGETS foe_position
  EXPORT foe_position
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_position
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_position-config.cmake DESTINATION share/cmake/foe)
