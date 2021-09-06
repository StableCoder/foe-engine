cmake_minimum_required(VERSION 3.13)
project(foe_position_yaml)

# Declaration
add_library(foe_position_yaml)

# Definition
generate_export_header(
  foe_position_yaml EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/position/yaml/export.h)

target_include_directories(foe_position_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_position_yaml PUBLIC foe_imex_yaml foe_position)

if(UNIX
   OR CMAKE_C_COMPILER_ID MATCHES "[Cc]lang"
   OR CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang")
  target_compile_options(foe_position_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(foe_position_yaml PRIVATE /EHa)
endif()

target_code_coverage(foe_position_yaml)

add_subdirectory(src)
