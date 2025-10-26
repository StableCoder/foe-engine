# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(assimp REQUIRED)
find_dependency(foe REQUIRED COMPONENTS model)

include(${CMAKE_CURRENT_LIST_DIR}/foe_model_assimp.cmake)
