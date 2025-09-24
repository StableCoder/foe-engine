# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_imex_binary)

# Declaration
add_library(foe_imex_binary SHARED)
add_library(foe::imex::binary ALIAS foe_imex_binary)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_imex_binary PUBLIC foe_imex foe_simulation)

target_code_coverage(foe_imex_binary)

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_imex_binary PROPERTY EXPORT_NAME imex::binary)

install(
  TARGETS foe_imex_binary
  EXPORT foe_imex_binary
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_imex_binary
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_imex_binary-config.cmake DESTINATION share/cmake/foe)
