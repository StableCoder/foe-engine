# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_resource_binary
  PRIVATE export_registration.c
          image.c
          material.c
          mesh.c
          shader.c
          vertex_descriptor.c
          result.c)
