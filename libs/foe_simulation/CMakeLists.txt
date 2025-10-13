# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_simulation)

# Declaration
add_library(foe_simulation SHARED)
add_library(foe::simulation ALIAS foe_simulation)

# Definition
generate_export_header(
  foe_simulation EXPORT_MACRO_NAME FOE_SIM_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/simulation/export.h)

target_link_libraries(foe_simulation PUBLIC foe_core foe_ecs foe_graphics
                                            foe_imex foe_resource)

target_code_coverage(foe_simulation)

if(MSVC)
  target_compile_options(foe_simulation PRIVATE /EHsc)
else()
  target_compile_options(foe_simulation PRIVATE -fno-exceptions)
endif()

target_sources(
  foe_simulation
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/simulation/export.h
         include/foe/simulation/group_data.hpp
         include/foe/simulation/registration.hpp
         include/foe/simulation/resource_create_info_history.h
         include/foe/simulation/resource_create_info_pool.h
         include/foe/simulation/result.h
         include/foe/simulation/simulation.hpp
         include/foe/simulation/type_defs.h)

add_subdirectory(src)

# Auxiliary Targets
add_subdirectory(libs/imgui)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_simulation PROPERTY EXPORT_NAME simulation)

install(
  TARGETS foe_simulation
  EXPORT foe_simulation
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_simulation
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_simulation-config.cmake DESTINATION share/cmake/foe)
