cmake_minimum_required(VERSION 3.13)
project(foe_yaml)

# Dependencies
find_path(YAML_CPP_INCLUDE_DIR NAMES yaml-cpp/yaml.h)
mark_as_advanced(YAML_CPP_INCLUDE_DIR)
find_library(YAML_CPP_LIBRARY NAMES yaml-cpp yaml-cppd)
mark_as_advanced(YAML_CPP_LIBRARY)

# Declaration
add_library(foe_yaml)

# Definition
generate_export_header(foe_yaml EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/yaml/export.h)

target_include_directories(foe_yaml PUBLIC include ${YAML_CPP_INCLUDE_DIR}
                                           ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_yaml PUBLIC ${YAML_CPP_LIBRARY})
target_code_coverage(foe_yaml)

add_subdirectory(src)
