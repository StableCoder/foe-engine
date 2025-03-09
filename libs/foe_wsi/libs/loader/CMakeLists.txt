# Copyright (C) 2022-2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.23)
project(foe_wsi_loader)

# Declaration
add_library(foe_wsi_loader)
set_property(TARGET foe_wsi_loader PROPERTY EXPORT_NAME wsi_loader)

# Definition
add_subdirectory(include)
add_subdirectory(src)

target_link_libraries(foe_wsi_loader PUBLIC foe_wsi)

target_code_coverage(foe_wsi_loader)

if(MSVC)
  target_compile_options(foe_wsi_loader PRIVATE /EHsc)
else()
  target_compile_options(foe_wsi_loader PRIVATE -fno-exceptions)
endif()

if(WSI_LOADER)
  # If set to use the WSI 'loader', then define an inherited flag that indicates
  # for any downstream projects to add the code for loading WSI implementations
  # dynamically (via foeWsiLoadImplementation) and where the default
  # implementation to load is (if implemented)
  target_compile_definitions(
    foe_wsi_loader
    PUBLIC WSI_LOADER
           DEFAULT_WSI_IMPLEMENTATION="$<TARGET_FILE:foe_wsi_${WSI_LIBRARY}>")
else()

endif()
