# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

include(CMakeFindDependencyMacro)
find_dependency(foe REQUIRED COMPONENTS xr)
find_dependency(glm REQUIRED)
find_dependency(OpenXR REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/foe_xr_openxr.cmake)
