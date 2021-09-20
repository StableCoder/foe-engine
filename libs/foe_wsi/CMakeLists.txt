cmake_minimum_required(VERSION 3.15)
project(foe_wsi)

# Options
option(BUILD_WINDOW_TESTS
       "Enables building tests that utilize Window subsystems" OFF)

# Dependencies
find_package(Vulkan REQUIRED)

# Declaration
add_library(foe_wsi INTERFACE)

# Definition
target_include_directories(foe_wsi INTERFACE include ${glfw_INCLUDE_DIR})

target_link_libraries(foe_wsi INTERFACE foe Vulkan::Vulkan)

# Auxiliary Targets
add_subdirectory(libs)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
