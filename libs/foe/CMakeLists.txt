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
target_code_coverage(foe)

if(UNIX)
  target_compile_options(foe PRIVATE -fPIC)
  target_link_libraries(foe PUBLIC dl pthread)
endif()

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_EXTRAS)
  add_subdirectory(extras)
endif()
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
