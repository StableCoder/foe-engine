# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_shared
  PRIVATE ../log.cpp
          ../error_code.c
          ../vk_result.c
          armature_create_info.cpp
          armature_loader.cpp
          armature_system.cpp
          camera_system.cpp
          position_descriptor_pool.cpp
          registration.cpp
          vk_animation.cpp)

add_subdirectory(yaml)
