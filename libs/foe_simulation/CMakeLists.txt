cmake_minimum_required(VERSION 3.15)
project(foe_simulation)

# Declaration
add_library(foe_simulation SHARED)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_simulation EXPORT_MACRO_NAME FOE_SIM_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/simulation/export.h)

target_include_directories(foe_simulation
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_simulation PUBLIC foe foe_ecs foe_graphics foe_imex
                                            foe_resource)

target_code_coverage(foe_simulation)

if(MSVC)
  target_compile_options(foe_simulation PRIVATE /EHsc)
else()
  target_compile_options(foe_simulation PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
