# Copyright (C) 2022-2023 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_bringup_shared
  PRIVATE ../log.cpp
          ../result.c
          ../vk_result.c
          armature_loader.cpp
          armature_state.cpp
          armature_system.cpp
          binary.cpp
          cleanup.c
          compare.c
          registration.cpp
          render_system_armature.cpp
          render_system_position.cpp
          render_system.cpp)

add_subdirectory(binary)
add_subdirectory(yaml)
