# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_shared
  PRIVATE ../log.cpp
          ../result.c
          ../vk_result.c
          armature_loader.cpp
          armature_system.cpp
          camera_system.cpp
          cleanup.c
          compare.c
          position_descriptor_pool.cpp
          registration.cpp
          vk_animation.cpp)

add_subdirectory(yaml)
