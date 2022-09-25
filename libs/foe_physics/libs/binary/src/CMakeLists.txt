# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_physics_binary
  PRIVATE collision_shape.c collision_shape.cpp export_registration.c
          import_registration.c result.c rigid_body.cpp)
