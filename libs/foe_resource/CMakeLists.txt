# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_resource)

# Declaration
add_library(foe_resource SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_resource PUBLIC foe_core foe_ecs)

target_code_coverage(foe_resource)

if(MSVC)
  target_compile_options(foe_resource PRIVATE /EHsc)
else()
  target_compile_options(foe_resource PRIVATE -fno-exceptions)
endif()

# Other
add_subdirectory(libs/imgui)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_resource PROPERTY EXPORT_NAME resource)

install(
  TARGETS foe_resource
  EXPORT foe_resource
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_resource
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_resource-config.cmake DESTINATION share/cmake/foe)
