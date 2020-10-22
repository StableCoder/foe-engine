cmake_minimum_required(VERSION 3.13)
project(foe_wsi)

# Declaration
add_library(foe_wsi)

# Sources
add_subdirectory(src)

target_include_directories(foe_wsi PUBLIC include)
target_link_libraries(foe_wsi PUBLIC glfw)
target_code_coverage(foe_wsi)
