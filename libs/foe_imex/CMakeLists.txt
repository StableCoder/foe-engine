cmake_minimum_required(VERSION 3.15)
project(foe_imex)

# Declaration
add_library(foe_imex)

# Definition
add_subdirectory(src)

generate_export_header(foe_imex EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/imex/export.h)

target_include_directories(foe_imex PUBLIC include
                                           ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_imex PUBLIC foe_ecs)

target_code_coverage(foe_imex)

if(MSVC)
  target_compile_options(foe_imex PRIVATE /EHsc)
else()
  target_compile_options(foe_imex PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs/yaml)
