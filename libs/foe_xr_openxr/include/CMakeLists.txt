# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

generate_export_header(
  foe_xr_openxr EXPORT_MACRO_NAME FOE_XR_EXPORT EXPORT_FILE_NAME
  ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/export.h)

target_sources(
  foe_xr_openxr
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         ${CMAKE_CURRENT_BINARY_DIR}/foe/xr/export.h
         foe/xr/openxr/camera_math.hpp
         foe/xr/openxr/result.h
         foe/xr/openxr/runtime.h
         foe/xr/openxr/session.h)
