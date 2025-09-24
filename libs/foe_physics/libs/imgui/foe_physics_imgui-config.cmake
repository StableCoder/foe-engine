# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(
  foe
  REQUIRED
  COMPONENTS
  imgui
  physics
  resource::imgui
  simulation::imgui)

include(${CMAKE_CURRENT_LIST_DIR}/foe_physics_imgui.cmake)
