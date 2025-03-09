# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_wsi
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/
         FILES
         foe/wsi/keyboard.hpp
         foe/wsi/mouse.hpp
         foe/wsi/result.h
         foe/wsi/vulkan.h
         foe/wsi/window.h)
