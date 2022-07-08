# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_xr_openxr
  PRIVATE camera_math.cpp
          core.cpp
          debug_utils.cpp
          error_code.c
          log.cpp
          runtime.cpp
          session.cpp
          xr_result.c)
