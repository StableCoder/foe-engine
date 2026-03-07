# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_core
  PRIVATE binary_result.c
          delimited_string.c
          filesystem.cpp
          hex.c
          log.cpp
          logger.cpp
          managed_memory_subset.c
          managed_memory.cpp
          plugin.cpp
          result.c
          search_paths_reader.cpp
          search_paths_writer.cpp
          search_paths.cpp
          split_thread_pool.cpp
          utf_character_conversion.c
          utf_string_conversion.c)

if(WIN32)
  target_sources(foe_core PRIVATE memory_mapped_file_win32.cpp)
else()
  target_sources(foe_core PRIVATE memory_mapped_file_posix.c)
endif()

add_subdirectory(chrono)
