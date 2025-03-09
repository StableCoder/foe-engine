# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_wsi_imgui)

# Declaration
add_library(foe_wsi_imgui)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_wsi_imgui PUBLIC foe_imgui)

target_code_coverage(foe_wsi_imgui)

if(MSVC)
  target_compile_options(foe_wsi_imgui PRIVATE /EHsc)
else()
  target_compile_options(foe_wsi_imgui PRIVATE -fno-exceptions)
endif()

if(WSI_LOADER)
  target_link_libraries(foe_wsi_imgui PUBLIC foe_wsi_loader)
else()
  target_link_libraries(foe_wsi_imgui PUBLIC foe_wsi_${WSI_LIBRARY})
endif()
