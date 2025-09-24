# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_model)

# Dependencies
find_package(glm REQUIRED)

# Declarations
add_library(foe_model)
add_library(foe::model ALIAS foe_model)

# Library Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_model PUBLIC foe_core)

target_code_coverage(foe_model)

if(DEFINED GLM_INCLUDE_DIRS)
  target_include_directories(foe_model PUBLIC ${GLM_INCLUDE_DIRS}
                                              ${GLM_INCLUDE_DIRS}/include)
elseif(TARGET glm::glm)
  target_link_libraries(foe_model PUBLIC glm::glm)
else()
  target_link_libraries(foe_model PUBLIC glm::glm-header-only)
endif()

if(UNIX)
  target_compile_options(foe_model PRIVATE -fPIC)
endif()

if(MSVC)
  target_compile_options(foe_model PRIVATE /EHsc)
else()
  target_compile_options(foe_model PRIVATE -fno-exceptions)
endif()

# install
set_property(TARGET foe_model PROPERTY EXPORT_NAME model)

install(
  TARGETS foe_model
  EXPORT foe_model
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_model
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_model-config.cmake DESTINATION share/cmake/foe)
