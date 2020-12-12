cmake_minimum_required(VERSION 3.13)
project(foe)

# Dependencies
find_package(fmt REQUIRED)

# Declaration
add_library(foe)

# Definition
generate_export_header(foe EXPORT_MACRO_NAME FOE_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/export.h)

target_include_directories(foe PUBLIC include
                                      ${CMAKE_CURRENT_BINARY_DIR}/exports)

target_link_libraries(foe PUBLIC fmt::fmt)
if(UNIX)
  target_link_libraries(foe PRIVATE dl)
endif()

target_code_coverage(foe)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
