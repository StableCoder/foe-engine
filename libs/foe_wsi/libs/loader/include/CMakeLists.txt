# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

if(WSI_LOADER)
  # Export the WSI_EXPORT header from this library
  generate_export_header(
    foe_wsi_loader EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)

  target_sources(
    foe_wsi_loader
    PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR} FILES
           ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)
else()
  # Don't export the WSI_EXPORT header from this library, local use only
  generate_export_header(
    foe_wsi_loader EXPORT_MACRO_NAME FOE_WSI_EXPORT EXPORT_FILE_NAME
    ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)

  target_sources(
    foe_wsi_loader
    PRIVATE FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/private
            FILES ${CMAKE_CURRENT_BINARY_DIR}/foe/wsi/export.h)
endif()

target_sources(
  foe_wsi_loader PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/
                        FILES foe/wsi/loader.h)
