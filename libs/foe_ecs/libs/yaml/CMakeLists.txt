cmake_minimum_required(VERSION 3.13)
project(foe_ecs_yaml CXX)

# Declaration
add_library(foe_ecs_yaml)

# Definition
generate_export_header(
  foe_ecs_yaml EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/ecs/yaml/export.h)

target_include_directories(foe_ecs_yaml
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe_ecs_yaml PUBLIC foe_ecs foe_yaml)

if(UNIX)
  target_compile_options(foe_ecs_yaml PRIVATE -fexceptions)
else(MSVC)
  target_compile_options(foe_ecs_yaml PRIVATE /EHa)
endif()

target_code_coverage(foe_ecs_yaml)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
