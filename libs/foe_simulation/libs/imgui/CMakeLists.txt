cmake_minimum_required(VERSION 3.15)
project(foe_simulation_imgui)

# Declaration
add_library(foe_simulation_imgui)

# Definition
add_subdirectory(src)

generate_export_header(
  foe_simulation_imgui EXPORT_MACRO_NAME FOE_SIM_IMGUI_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/public/foe/simulation/imgui/export.h)

target_include_directories(foe_simulation_imgui
                           PUBLIC include ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_simulation_imgui PUBLIC foe_imgui foe_simulation)

target_code_coverage(foe_simulation_imgui)

if(MSVC)
  target_compile_options(foe_simulation_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_simulation_imgui PRIVATE -fno-exceptions)
endif()
