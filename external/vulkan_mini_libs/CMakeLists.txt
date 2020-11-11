cmake_minimum_required(VERSION 3.13)

# Dependencies
find_package(Vulkan REQUIRED)

# Common
if(UNIX OR APPLE)
  add_compile_options(-fPIC)
endif()

# Equality Checks
add_library(vk_equality_checks STATIC src/equality_checks.cpp)

target_include_directories(vk_equality_checks PUBLIC vulkan-mini-libs/include)
target_link_libraries(vk_equality_checks PUBLIC Vulkan::Vulkan)

# Error Code
add_library(vk_error_code STATIC src/error_code.cpp)

target_include_directories(vk_error_code PUBLIC vulkan-mini-libs/include)
target_link_libraries(vk_error_code PUBLIC Vulkan::Vulkan)

# Value Serialization
add_library(vk_value_serialization STATIC src/value_serialization.cpp)

target_include_directories(vk_value_serialization
                           PUBLIC vulkan-mini-libs/include)
target_link_libraries(vk_value_serialization PUBLIC Vulkan::Vulkan)
