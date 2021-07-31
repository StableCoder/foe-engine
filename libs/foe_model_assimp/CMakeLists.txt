cmake_minimum_required(VERSION 3.13)
project(foe_model_assimp)

# Dependencies
find_package(assimp REQUIRED)

# Declaration
add_library(foe_model_assimp)

# Definition
generate_export_header(
  foe_model_assimp EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/model/assimp/export.h)

target_include_directories(foe_model_assimp
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_model_assimp PUBLIC assimp::assimp foe_model)
target_code_coverage(foe_model_assimp)

add_subdirectory(src)

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
