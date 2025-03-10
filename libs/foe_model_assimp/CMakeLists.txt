# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_model_assimp)

# Dependencies
if(UNIX)
  # Assimp 5.0 CMake config files are broken on Linux, use the pkgconf ones
  find_package(PkgConfig)
  pkg_check_modules(assimp REQUIRED assimp)
else()
  find_package(assimp REQUIRED)
endif()

# Declaration
add_library(foe_model_assimp SHARED)
set_property(TARGET foe_model_assimp PROPERTY EXPORT_NAME model_assimp)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_include_directories(foe_model_assimp PRIVATE ${assimp_INCLUDE_DIRS})

target_link_libraries(foe_model_assimp PUBLIC foe_model
                                              ${assimp_LINK_LIBRARIES})
if(NOT UNIX)
  target_link_libraries(foe_model_assimp PUBLIC assimp::assimp)
endif()

target_code_coverage(foe_model_assimp)

if(MSVC)
  target_compile_options(foe_model_assimp PRIVATE /EHsc)
else()
  target_compile_options(foe_model_assimp PRIVATE -fno-exceptions)
endif()

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
install(
  TARGETS foe_model_assimp
  EXPORT foe_model_assimp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_model_assimp
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_model_assimp-config.cmake DESTINATION share/cmake/foe)
