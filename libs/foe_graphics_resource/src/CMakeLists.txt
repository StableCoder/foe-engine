# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_resource
  PRIVATE image_create_info.cpp
          image_loader.cpp
          log.cpp
          material_create_info.cpp
          material_loader.cpp
          mesh_create_info.cpp
          mesh_loader.cpp
          registration.cpp
          result.c
          shader_create_info.cpp
          shader_loader.cpp
          vertex_descriptor_create_info.cpp
          vertex_descriptor_loader.cpp
          worst_resource_state.cpp
          vertex_descriptor.cpp
          vk_result.c)
