# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(foe REQUIRED COMPONENTS imgui simulation)

include(${CMAKE_CURRENT_LIST_DIR}/foe_simulation_imgui.cmake)
