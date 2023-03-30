# Copyright (C) 2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe_network)

# Declaration
add_library(foe_network SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_network EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/network/export.h)

target_include_directories(foe_network
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_network PUBLIC foe)
if(WIN32)
  target_link_libraries(foe_network PUBLIC ws2_32)
endif()

target_code_coverage(foe_network)

if(MSVC)
  target_compile_options(foe_network PRIVATE /EHsc)
else()
  target_compile_options(foe_network PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
