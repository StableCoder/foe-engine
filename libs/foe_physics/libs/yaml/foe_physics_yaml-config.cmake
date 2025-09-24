# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(
  foe
  REQUIRED
  COMPONENTS
  ecs::yaml
  imex::yaml
  physics
  yaml)

include(${CMAKE_CURRENT_LIST_DIR}/foe_physics_yaml.cmake)
