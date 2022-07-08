# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_resource
  PRIVATE create_info.cpp
          error_code.c
          load_state.cpp
          log.cpp
          pool.cpp
          records.cpp
          resource.cpp)
