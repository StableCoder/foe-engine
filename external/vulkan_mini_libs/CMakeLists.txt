cmake_minimum_required(VERSION 3.13)

# Dependencies
find_package(Vulkan REQUIRED)

# Common
if(UNIX OR APPLE)
  add_compile_options(-fPIC)
endif()

# Interface Library
add_library(khronos_mini_libs INTERFACE)

target_include_directories(khronos_mini_libs
                           INTERFACE vulkan-mini-libs-2/include)

# Struct Cleanup
add_library(vk_struct_cleanup STATIC src/struct_cleanup.c)

target_include_directories(vk_struct_cleanup PUBLIC vulkan-mini-libs-2/include)
target_link_libraries(vk_struct_cleanup PUBLIC Vulkan::Vulkan)

# Struct Cleanup
add_library(vk_struct_compare STATIC src/struct_compare.c)

target_include_directories(vk_struct_compare PUBLIC vulkan-mini-libs-2/include)
target_link_libraries(vk_struct_compare PUBLIC Vulkan::Vulkan)

# Value Serialization
add_library(vk_value_serialization STATIC src/value_serialization.cpp)

target_include_directories(vk_value_serialization
                           PUBLIC vulkan-mini-libs-2/include)
target_link_libraries(vk_value_serialization PUBLIC Vulkan::Vulkan)
