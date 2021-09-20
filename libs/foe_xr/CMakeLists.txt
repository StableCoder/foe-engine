cmake_minimum_required(VERSION 3.15)
project(foe_xr)

# Declaration
add_library(foe_xr INTERFACE)

# Definition
target_compile_definitions(foe_xr INTERFACE FOE_XR_SUPPORT)

target_include_directories(foe_xr INTERFACE include)

target_link_libraries(foe_xr INTERFACE foe)
