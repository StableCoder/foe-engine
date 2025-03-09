# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_xr
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         foe/xr/runtime.h
         foe/xr/session.h)
