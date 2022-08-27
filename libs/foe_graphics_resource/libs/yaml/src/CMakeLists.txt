# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_graphics_resource_yaml
  PRIVATE export_registration.cpp
          image.cpp
          import_registration.cpp
          material.cpp
          mesh.cpp
          result.c
          shader.cpp
          structs.cpp
          vertex_descriptor.cpp)
