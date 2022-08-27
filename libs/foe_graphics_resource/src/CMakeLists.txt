# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_resource
  PRIVATE binary.c
          cleanup.c
          compare.c
          image_loader.cpp
          log.cpp
          material_loader.cpp
          mesh_loader.cpp
          registration.cpp
          result.c
          shader_loader.cpp
          vertex_descriptor_loader.cpp
          worst_resource_state.cpp
          vertex_descriptor.cpp
          vk_result.c)
