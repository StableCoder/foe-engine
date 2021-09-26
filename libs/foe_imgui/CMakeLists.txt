cmake_minimum_required(VERSION 3.15)
project(foe_imgui)

# Declaration
add_library(foe_imgui)

# Definition
add_subdirectory(src)

generate_export_header(foe_imgui EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/public/foe/imgui/export.h)

target_include_directories(foe_imgui PUBLIC include
                                            ${CMAKE_CURRENT_BINARY_DIR}/public)

target_link_libraries(foe_imgui PUBLIC foe_graphics_vk foe_wsi imgui
                                       imgui_file_dialog)

target_code_coverage(foe_imgui)

if(MSVC)
  target_compile_options(foe_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_imgui PRIVATE -fno-exceptions)
endif()

# Auxiliary Target
if(BUILD_TESTS)
  add_subdirectory(test)
endif()
