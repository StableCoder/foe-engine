# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(foe REQUIRED COMPONENTS ecs imex::binary physics)

include(${CMAKE_CURRENT_LIST_DIR}/foe_physics_binary.cmake)
