cmake_minimum_required(VERSION 3.13)
project(foe_position)

# Dependencies
find_package(glm REQUIRED)

# Declaration
add_library(foe_position INTERFACE)

# Definition
target_include_directories(foe_position INTERFACE include)
target_link_libraries(foe_position INTERFACE foe foe_imex glm)
target_code_coverage(foe_position INTERFACE)

# Auxiliary Targets
add_subdirectory(libs/yaml)
