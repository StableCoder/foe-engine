cmake_minimum_required(VERSION 3.15)
project(foe_yaml)

# Dependencies
find_package(yamlcpp REQUIRED)

# Declaration
add_library(foe_yaml SHARED)

# Definition
add_subdirectory(src)

generate_export_header(foe_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/yaml/export.h)

target_include_directories(foe_yaml PUBLIC include
                                           ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_yaml PUBLIC yamlcpp::yamlcpp)

target_code_coverage(foe_yaml)

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
