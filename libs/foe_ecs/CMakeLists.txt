cmake_minimum_required(VERSION 3.15)
project(foe_ecs)

# Declaration
add_library(foe_ecs)

# Definition
add_subdirectory(src)

generate_export_header(foe_ecs EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/ecs/export.h)

target_include_directories(foe_ecs PUBLIC include
                                          ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe_ecs PUBLIC foe)

target_code_coverage(foe_ecs)

if(MSVC)
  target_compile_options(foe_ecs PRIVATE /EHsc)
else()
  target_compile_options(foe_ecs PRIVATE -fno-exceptions)
endif()

# Auxiliary Targets
add_subdirectory(libs)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
