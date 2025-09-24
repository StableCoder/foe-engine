# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(foe COMPONENTS ecs resource)

include(${CMAKE_CURRENT_LIST_DIR}/foe_imex.cmake)
