# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 0)

add_subdirectory(CLI11)
add_subdirectory(imgui)
add_subdirectory(imgui_file_dialog)
add_subdirectory(vulkan_memory_allocator)
add_subdirectory(vulkan_mini_libs)
