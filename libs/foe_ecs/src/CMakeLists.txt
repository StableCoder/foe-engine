# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_ecs PRIVATE error_code.c group_translator.cpp id_to_string.cpp
                  indexes.cpp log.cpp name_map.cpp)
