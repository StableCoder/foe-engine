# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe
  PRIVATE command_string_runner.cpp
          delimited_string.c
          developer_console.cpp
          filesystem.cpp
          log.cpp
          managed_memory.cpp
          plugin.cpp
          result.c
          search_paths_reader.cpp
          search_paths_writer.cpp
          search_paths.cpp
          split_thread_pool.cpp
          thread_pool_result.c
          thread_pool.cpp)

add_subdirectory(chrono)
add_subdirectory(log)
