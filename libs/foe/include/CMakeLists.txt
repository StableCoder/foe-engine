# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0
generate_export_header(foe_core EXPORT_MACRO_NAME FOE_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/foe/export.h)

target_sources(
  foe_core
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/
         ${CMAKE_CURRENT_BINARY_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/export.h
         foe/chrono/dilated_clock.hpp
         foe/chrono/dilated_long_clock.hpp
         foe/chrono/easy_clock.hpp
         foe/chrono/program_clock.hpp
         foe/algorithm.hpp
         foe/binary_result.h
         foe/delimited_string.h
         foe/engine_detail.h
         foe/filesystem.hpp
         foe/handle.h
         foe/hex.h
         foe/log.h
         foe/managed_memory.h
         foe/memory_alignment.h
         foe/memory_mapped_file.h
         foe/plugin.h
         foe/quaternion_math.hpp
         foe/result.h
         foe/search_paths.hpp
         foe/split_thread_pool.h
         foe/type_defs.h)
