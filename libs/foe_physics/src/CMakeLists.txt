# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_physics
  PRIVATE collision_shape_create_info.cpp collision_shape_loader.cpp log.cpp
          registration.cpp result.c system.cpp)
