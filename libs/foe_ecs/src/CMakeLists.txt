# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_ecs
  PRIVATE binary.c
          component_pool.cpp
          group_translator.cpp
          id_to_string.cpp
          indexes.cpp
          log.cpp
          name_map.cpp
          result.c)
