# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_resource_binary
  PRIVATE export_registration.c
          image.c
          image.cpp
          import_registration.c
          material.c
          material.cpp
          mesh.c
          mesh.cpp
          shader.c
          shader.cpp
          vertex_descriptor.c
          vertex_descriptor.cpp
          result.c)
