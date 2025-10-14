# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13)

# Dependencies
find_package(Vulkan REQUIRED)
if(XR_SUPPORT)
  find_package(OpenXR REQUIRED)
endif()

# Common
if(UNIX OR APPLE)
  add_compile_options(-fPIC)
endif()

# Struct Cleanup
add_library(vk_struct_cleanup STATIC)
add_library(foe::vk_struct_cleanup ALIAS vk_struct_cleanup)

set_property(TARGET vk_struct_cleanup PROPERTY EXPORT_NAME
                                               foe::vk_struct_cleanup)

target_include_directories(
  vk_struct_cleanup
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include>)
target_link_libraries(vk_struct_cleanup PUBLIC Vulkan::Vulkan)

target_sources(
  vk_struct_cleanup
  PUBLIC
    FILE_SET HEADERS BASE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include/vk_struct_cleanup.h
  PRIVATE src/struct_cleanup.c)

# Struct Cleanup
add_library(vk_struct_compare STATIC)
add_library(foe::vk_struct_compare ALIAS vk_struct_compare)

set_property(TARGET vk_struct_compare PROPERTY EXPORT_NAME
                                               foe::vk_struct_compare)

target_include_directories(
  vk_struct_compare
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include>)
target_link_libraries(vk_struct_compare PUBLIC Vulkan::Vulkan)

target_sources(
  vk_struct_compare
  PUBLIC
    FILE_SET HEADERS BASE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include/vk_struct_compare.h
  PRIVATE src/struct_compare.c)

# VK Value Serialization
add_library(vk_value_serialization STATIC)
add_library(foe::vk_value_serialization ALIAS vk_value_serialization)

set_property(TARGET vk_value_serialization PROPERTY EXPORT_NAME
                                                    foe::vk_value_serialization)

target_include_directories(
  vk_value_serialization
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include>)
target_link_libraries(vk_value_serialization PUBLIC Vulkan::Vulkan)

target_sources(
  vk_value_serialization
  PUBLIC
    FILE_SET
    HEADERS
    BASE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include/vk_value_serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include/vk_value_serialization.hpp
  PRIVATE src/value_serialization.cpp)

# Vulkan Result Serialization
add_library(vk_result_serialization INTERFACE)
add_library(foe::vk_result_serialization ALIAS vk_result_serialization)

set_property(TARGET vk_result_serialization
             PROPERTY EXPORT_NAME foe::vk_result_serialization)

target_include_directories(
  vk_result_serialization
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include>)
target_link_libraries(vk_result_serialization INTERFACE Vulkan::Vulkan)

target_sources(
  vk_result_serialization
  INTERFACE
    FILE_SET HEADERS BASE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include/vk_result_to_string.h
)

# OpenXR Result Serialization
if(XR_SUPPORT)
  add_library(xr_result_serialization INTERFACE)
  add_library(foe::xr_result_serialization ALIAS xr_result_serialization)

  set_property(TARGET xr_result_serialization
               PROPERTY EXPORT_NAME foe::xr_result_serialization)

  target_include_directories(
    xr_result_serialization
    INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include>)
  target_link_libraries(xr_result_serialization INTERFACE OpenXR::headers)

  target_sources(
    xr_result_serialization
    INTERFACE
      FILE_SET
      HEADERS
      BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include
      FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/vulkan-mini-libs-2/include/xr_result_to_string.h
  )
endif()
