# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

# Dependencies
find_package(Vulkan REQUIRED)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Library
add_library(vulkan_memory_allocator)
add_library(foe::vulkan_memory_allocator ALIAS vulkan_memory_allocator)

target_include_directories(
  vulkan_memory_allocator PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
target_link_libraries(vulkan_memory_allocator PUBLIC Vulkan::Vulkan)
if(UNIX)
  target_compile_options(vulkan_memory_allocator PRIVATE -fPIC)
endif()

target_sources(
  vulkan_memory_allocator
  # @TODO Move VMA header to sub-directory to avoid clashing
  PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/ FILES
         ${CMAKE_CURRENT_SOURCE_DIR}/vk_mem_alloc.h
  PRIVATE vk_mem_alloc.cpp)
