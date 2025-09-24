# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_ecs)

# Declaration
add_library(foe_ecs SHARED)
add_library(foe::ecs ALIAS foe_ecs)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_ecs PUBLIC foe_core)

target_code_coverage(foe_ecs)

if(MSVC)
  target_compile_options(foe_ecs PRIVATE /EHsc)
else()
  target_compile_options(foe_ecs PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/yaml)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_ecs PROPERTY EXPORT_NAME ecs)

install(
  TARGETS foe_ecs
  EXPORT foe_ecs
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_ecs
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_ecs-config.cmake DESTINATION share/cmake/foe)
