cmake_minimum_required(VERSION 3.13)
project(foe_simulation)

# Declaration
add_library(foe_simulation)

# Definition
generate_export_header(
  foe_simulation EXPORT_MACRO_NAME FOE_SIM_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/simulation/export.h)

target_include_directories(foe_simulation
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_simulation PUBLIC foe foe_ecs foe_graphics foe_imex)

target_code_coverage(foe_simulation)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
