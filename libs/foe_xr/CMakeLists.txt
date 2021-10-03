cmake_minimum_required(VERSION 3.15)
project(foe_xr)

# Declaration
add_library(foe_xr INTERFACE)

# Definition
if(XR_SUPPORT)
  target_compile_definitions(foe_xr INTERFACE FOE_XR_SUPPORT)
endif()

target_include_directories(foe_xr INTERFACE include)

target_link_libraries(foe_xr INTERFACE foe)
