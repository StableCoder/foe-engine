cmake_minimum_required(VERSION 3.13)
project(foe_yaml)

# Dependencies
find_package(yamlcpp REQUIRED)

# Declaration
add_library(foe_yaml)

# Definition
generate_export_header(foe_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/yaml/export.h)

target_include_directories(foe_yaml PUBLIC include
                                           ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_yaml PUBLIC yamlcpp::yamlcpp)
target_code_coverage(foe_yaml)

add_subdirectory(src)

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
