# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe
  PRIVATE binary_result.c
          command_string_runner.cpp
          delimited_string.c
          developer_console.cpp
          filesystem.cpp
          log.cpp
          managed_memory_subset.c
          managed_memory.cpp
          plugin.cpp
          result.c
          search_paths_reader.cpp
          search_paths_writer.cpp
          search_paths.cpp
          split_thread_pool.cpp
          thread_pool_result.c
          thread_pool.cpp)

if(WIN32)
  target_sources(foe PRIVATE memory_mapped_file_win32.cpp)
else()
  target_sources(foe PRIVATE memory_mapped_file_posix.c)
endif()

add_subdirectory(chrono)
add_subdirectory(log)
