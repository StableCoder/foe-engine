# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)
project(foe)

# Dependencies
find_package(fmt REQUIRED)

# Declaration
add_library(foe SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe EXPORT_MACRO_NAME FOE_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/export.h)

target_include_directories(foe PUBLIC include
                                      ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe PUBLIC fmt::fmt)

target_code_coverage(foe)

if(DISABLE_PLUGIN_UNLOAD)
  target_compile_definitions(foe PRIVATE DISABLE_PLUGIN_UNLOAD)
endif()

if(UNIX)
  target_compile_options(foe PRIVATE -fPIC)
  target_link_libraries(foe PUBLIC dl pthread)
endif()

if(WIN32)
  # For timeBeginPeriod()/timeEndPeriod()
  target_link_libraries(foe PUBLIC winmm)
endif()

if(MSVC)
  target_compile_options(foe PRIVATE /EHsc)
else()
  target_compile_options(foe PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
