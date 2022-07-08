# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_yaml PRIVATE armature.cpp error_code.c export_registration.cpp
                           import_registration.cpp)

target_link_libraries(foe_bringup_yaml PRIVATE foe_model foe_position)
