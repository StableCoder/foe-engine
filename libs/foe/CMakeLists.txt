# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_core)

# Dependencies
find_package(fmt REQUIRED)

# Declaration
add_library(foe_core SHARED)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_core PUBLIC fmt::fmt)

target_code_coverage(foe_core)

if(DISABLE_PLUGIN_UNLOAD)
  target_compile_definitions(foe_core PRIVATE DISABLE_PLUGIN_UNLOAD)
endif()

if(UNIX)
  target_compile_options(foe_core PRIVATE -fPIC)
  target_link_libraries(foe_core PRIVATE dl pthread)
endif()

if(WIN32)
  # For timeBeginPeriod()/timeEndPeriod()
  target_link_libraries(foe_core PRIVATE winmm)
endif()

if(MSVC)
  target_compile_options(foe_core PRIVATE /EHsc)
else()
  target_compile_options(foe_core PRIVATE -fno-exceptions)
endif()

# test
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

# install
set_property(TARGET foe_core PROPERTY EXPORT_NAME core)

install(
  TARGETS foe_core
  EXPORT foe_core
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
          FILE_SET HEADERS
          DESTINATION include)

install(
  EXPORT foe_core
  NAMESPACE foe::
  DESTINATION share/cmake/foe)

install(FILES foe_core-config.cmake DESTINATION share/cmake/foe)
