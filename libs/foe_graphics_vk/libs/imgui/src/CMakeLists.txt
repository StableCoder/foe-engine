# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_vk_imgui PRIVATE fragment_descriptor.cpp shader.cpp
                                vertex_descriptor.cpp vk_struct.cpp vk_type.cpp)
