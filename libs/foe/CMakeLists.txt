# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_core)

# Dependencies
find_package(fmt REQUIRED)

# Declaration
add_library(foe_core SHARED)
add_library(foe::core ALIAS foe_core)

# Definition
generate_export_header(foe_core EXPORT_MACRO_NAME FOE_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/export.h)

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

target_sources(
  foe_core
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/export.h
         include/foe/chrono/dilated_clock.hpp
         include/foe/chrono/dilated_long_clock.hpp
         include/foe/chrono/easy_clock.hpp
         include/foe/chrono/program_clock.hpp
         include/foe/algorithm.hpp
         include/foe/binary_result.h
         include/foe/delimited_string.h
         include/foe/engine_detail.h
         include/foe/filesystem.hpp
         include/foe/handle.h
         include/foe/hex.h
         include/foe/log.h
         include/foe/managed_memory.h
         include/foe/memory_alignment.h
         include/foe/memory_mapped_file.h
         include/foe/plugin.h
         include/foe/quaternion_math.hpp
         include/foe/result.h
         include/foe/search_paths.hpp
         include/foe/split_thread_pool.h
         include/foe/type_defs.h)

add_subdirectory(src)

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
