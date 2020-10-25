cmake_minimum_required(VERSION 3.13)
project(foe)

# Dependencies
find_path(fmt_INCLUDE_DIRS NAMES fmt/format.h)
find_library(fmt_LIBRARIES fmt)
if(fmt_INCLUDE_DIRS-NOTFOUND OR fmt_LIBRARIES-NOTFOUND)
  message(FATAL_ERROR "Cannot find required FreeImage library!")
else()
  get_filename_component(fmt_INCLUDE_DIRS ${fmt_INCLUDE_DIRS} DIRECTORY)
endif()
if(WIN32 OR APPLE)
  set(fmt_INCLUDE_DIRS ${fmt_INCLUDE_DIRS}/include)
endif()
mark_as_advanced(FORCE fmt_INCLUDE_DIRS fmt_LIBRARIES)

# Declaration
add_library(foe)

# Definition
generate_export_header(foe EXPORT_MACRO_NAME FOE_EXPORT EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/exports/foe/export.h)

target_include_directories(foe PUBLIC include ${fmt_INCLUDE_DIRS}
                                      ${CMAKE_CURRENT_BINARY_DIR}/exports)
target_link_libraries(foe PUBLIC ${fmt_LIBRARIES})
target_code_coverage(foe)

add_subdirectory(src)

# Auxiliary Targets
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
