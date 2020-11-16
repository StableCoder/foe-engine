cmake_minimum_required(VERSION 3.13)

# Dependencies
find_package(Vulkan REQUIRED)

# Library
add_library(vulkan_memory_allocator STATIC vk_mem_alloc.cpp vk_mem_alloc.h)

target_include_directories(vulkan_memory_allocator
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vulkan_memory_allocator PUBLIC Vulkan::Vulkan)
if(UNIX)
  target_compile_options(vulkan_memory_allocator PRIVATE -fPIC)
endif()
