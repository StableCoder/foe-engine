cmake_minimum_required(VERSION 3.15)
project(foe_imex_yaml)

# Declaration
add_library(foe_imex_yaml)

# Definition
generate_export_header(
  foe_imex_yaml EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/imex/yaml/export.h)

target_include_directories(foe_imex_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_imex_yaml PUBLIC foe_imex foe_yaml foe_ecs_yaml
                                           foe_resource)

if(UNIX
   OR CMAKE_C_COMPILER_ID MATCHES "[Cc]lang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang")
  target_compile_options(foe_imex_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(foe_imex_yaml PRIVATE /EHa)
endif()

target_code_coverage(foe_imex_yaml)

add_subdirectory(src)

# Auxiliary Targets
