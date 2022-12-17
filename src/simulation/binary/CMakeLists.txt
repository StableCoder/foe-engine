# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_binary
  PRIVATE armature_create_info.c
          armature_create_info.cpp
          armature_state.cpp
          export_registration.c
          import_registration.c
          render_state.cpp
          result.c)

target_link_libraries(foe_bringup_binary PRIVATE foe_model foe_position)
