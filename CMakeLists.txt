cmake_minimum_required(VERSION 3.13)
project(FoE-Engine CXX)

# Modify CMake modules path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Global Options
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_EXTRAS "Builds other programs, such as benchmarks" OFF)
option(BUILD_SHADERS "Build GLSL shaders to SPIR-V automatically" ON)
option(BUILD_TESTS "Build test programs" OFF)
option(EDITOR_MODE "Build with the editor UI capability" OFF)
if(APPLE)
  option(BUILD_XR "Build with OpenXR support" OFF)
else()
  option(BUILD_XR "Build with OpenXR support" ON)
endif()

# C++20 standard, no extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Code Coverage
include(code-coverage)
add_code_coverage_all_targets(EXCLUDE .*/test/.*)

# Formatting
include(formatting)
file(
  GLOB_RECURSE
  CL_FILES
  *.[hc]
  *.[hc]pp
  *.vert
  *.tese
  *.tesc
  *.geom
  *.frag
  *.comp)
list(FILTER CL_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
list(FILTER CL_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(FILTER CL_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/external)
clang_format(format ${CL_FILES})

file(GLOB_RECURSE CM_FILES CMakeLists.txt *.cmake)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/external)
cmake_format(cmake-format ${CM_FILES})

# Dependency Graph
include(dependency-graph)
gen_dep_graph(png ADD_TO_DEP_GRAPH)

# clang-tidy
include(tools)
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
include(compiler-options)
include(glsl-shaders)
include(prepare-catch)
include(sanitizers)

if(WIN32 AND BUILD_SHARED_LIBS)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()
if(BUILD_TESTS)
  enable_testing()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW/Vulkan
add_compile_definitions(GLFW_INCLUDE_VULKAN)

# Libraries
add_subdirectory(external)
add_subdirectory(libs)

# Engine Entrypoint
add_executable(foe_engine)
target_link_libraries(foe_engine PRIVATE foe_engine_lib)
target_code_coverage(foe_engine)

# Engine Library
set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(foe_engine_lib)
target_link_libraries(
  foe_engine_lib
  PUBLIC CLI11
         foe
         foe_ecs
         foe_ecs_yaml
         foe_graphics_vk
         foe_imex
         foe_imex_yaml
         foe_model
         foe_physics
         foe_physics_yaml
         foe_position
         foe_position_yaml
         foe_resource
         foe_resource_yaml
         foe_simulation
         foe_wsi
         foe_xr)
target_code_coverage(foe_engine_lib)

if(BUILD_XR)
  target_link_libraries(foe_engine_lib PUBLIC foe_xr_openxr)
endif()

if(EDITOR_MODE)
  # Link to executable
  target_link_libraries(foe_engine_lib PUBLIC foe_engine_imgui)
endif()

# Engine ImGUI Library
add_library(foe_engine_imgui STATIC)
target_compile_definitions(foe_engine_imgui PUBLIC EDITOR_MODE)
target_link_libraries(foe_engine_imgui PUBLIC foe_imgui)

# Sources
add_subdirectory(src)
if(BUILD_SHADERS)
  file(
    GLOB_RECURSE
    GLSL_FILES
    data/*.vert
    data/*.tese
    data/*.tesc
    data/*.geom
    data/*.frag
    data/*.comp)

  target_glsl_shaders(foe_engine PRIVATE ${GLSL_FILES})
  clang_format(format_shaders ${GLSL_FILES})
endif()

# Auxiliary Items
