# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_simulation
  PRIVATE group_data.cpp
          log.cpp
          resource_create_info_history.cpp
          resource_create_info_pool.cpp
          result.c
          simulation.cpp
          struct.cpp)
