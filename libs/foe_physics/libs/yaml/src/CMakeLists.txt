# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_physics_yaml
  PRIVATE collision_shape.cpp export_registration.cpp import_registration.cpp
          result.c rigid_body.cpp structs.cpp)
