# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_skunkworks_binary
  PRIVATE armature_create_info.c
          armature_create_info.cpp
          armature_state.cpp
          export_registration.c
          import_registration.c
          render_state.cpp
          result.c)

target_link_libraries(foe_skunkworks_binary PRIVATE foe::model foe::position)
