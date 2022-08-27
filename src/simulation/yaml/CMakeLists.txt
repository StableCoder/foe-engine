# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_yaml PRIVATE armature.cpp export_registration.cpp
                           import_registration.cpp result.c structs.cpp)

target_link_libraries(foe_bringup_yaml PRIVATE foe_model foe_position)
