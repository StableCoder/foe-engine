# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_vk PRIVATE blit_image.cpp copy_image.cpp export_image.cpp
                          import_image.cpp present_image.cpp resolve_image.cpp)
