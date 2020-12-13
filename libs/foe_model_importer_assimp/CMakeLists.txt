cmake_minimum_required(VERSION 3.13)
project(foe_model_importer_assimp)

# Dependencies
find_package(assimp REQUIRED)

# Declaration
add_library(foe_model_importer_assimp)

# Definition
generate_export_header(foe_model_importer_assimp EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/export.h)

target_include_directories(foe_model_importer_assimp
                           PRIVATE include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_model_importer_assimp PRIVATE assimp::assimp
                                                        foe_model)

target_code_coverage(foe_model_importer_assimp)

add_subdirectory(src)

# Auxiliary Target
if(BUILD_TESTS AND EXISTS test)
  add_subdirectory(test)
endif()
