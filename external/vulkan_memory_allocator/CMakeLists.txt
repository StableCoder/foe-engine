# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

# Dependencies
find_package(Vulkan REQUIRED)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Library
add_library(vulkan_memory_allocator)
add_library(foe::external::vulkan_memory_allocator ALIAS
            vulkan_memory_allocator)

set_property(TARGET vulkan_memory_allocator
             PROPERTY EXPORT_NAME foe::external::vulkan_memory_allocator)

target_link_libraries(vulkan_memory_allocator PUBLIC Vulkan::Vulkan)
if(UNIX)
  target_compile_options(vulkan_memory_allocator PRIVATE -fPIC)
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/vk_mem_alloc.h
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h
          ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/vk_mem_alloc.h
  DEPENDS vk_mem_alloc.h)

target_sources(
  vulkan_memory_allocator
  PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include FILES
         ${CMAKE_CURRENT_BINARY_DIR}/include/foe/external/vk_mem_alloc.h
  PRIVATE vk_mem_alloc.cpp)
