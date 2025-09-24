# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(
  foe
  REQUIRED
  COMPONENTS
  ecs::yaml
  graphics::resource
  graphics::vk::yaml
  imex::yaml)

include(${CMAKE_CURRENT_LIST_DIR}/foe_graphics_resource_yaml.cmake)
