cmake_minimum_required(VERSION 3.13)
project(foe_wsi)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_wsi INTERFACE)

# Definition
target_include_directories(
  foe_wsi INTERFACE include ${glfw_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_wsi INTERFACE foe Vulkan::Vulkan)
target_code_coverage(foe_wsi)
