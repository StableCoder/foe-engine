cmake_minimum_required(VERSION 3.13)
project(FoE-Engine CXX)

# Global Options
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_TESTS "Build testcc programs" OFF)

# C++20 standard, no extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Code Coverage
include(cmake/code-coverage.cmake)
add_code_coverage_all_targets()

# Formatting
include(cmake/formatting.cmake)
file(GLOB_RECURSE CL_FILES *.[hc] *.[hc]pp)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
clang_format(format ${CL_FILES})

file(GLOB_RECURSE CM_FILES CMakeLists.txt *.cmake)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
cmake_format(cmake-format ${CM_FILES})

# Dependency Graph
include(cmake/dependency-graph.cmake)
gen_dep_graph(png ADD_TO_DEP_GRAPH)

# clang-tidy
include(cmake/tools.cmake)
option(CLANG_TIDY_FIX "" OFF)
if(CLANG_TIDY_FIX)
  clang_tidy(-header-filter='${CMAKE_SOURCE_DIR}/*' -fix)
else()
  clang_tidy(-header-filter='${CMAKE_SOURCE_DIR}/*')
endif()

# Header Exports
include(GenerateExportHeader)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Misc CMake
include(cmake/compiler-options.cmake)
include(cmake/prepare_catch.cmake)
include(cmake/sanitizers.cmake)

if(WIN32 AND BUILD_SHARED_LIBS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if(BUILD_TESTS)
  enable_testing()
endif()

# GLFW/Vulkan
add_compile_definitions(GLFW_INCLUDE_VULKAN)

# Libraries
add_subdirectory(libs)

# Engine Entrypoint
add_executable(foe_engine)
target_link_libraries(foe_engine PRIVATE foe foe_graphics foe_wsi)
target_code_coverage(foe_engine AUTO ALL EXTERNAL)

# Sources
add_subdirectory(src)
