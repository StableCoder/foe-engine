cmake_minimum_required(VERSION 3.15)
project(foe_model)

# Dependencies
find_package(glm REQUIRED)

# Declarations
add_library(foe_model_interface INTERFACE)
add_library(foe_model)

# Interface Definition
target_include_directories(
  foe_model_interface INTERFACE include ${CMAKE_CURRENT_BINARY_DIR}/exports)

# Library Definition
generate_export_header(foe_model EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/model/export.h)

target_link_libraries(foe_model PUBLIC foe foe_model_interface glm)

target_code_coverage(foe_model)

if(UNIX)
  target_compile_options(foe_model PRIVATE -fPIC)
endif()

add_subdirectory(src)

# Auxiliary Target
