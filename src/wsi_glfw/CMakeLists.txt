# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

find_package(glfw3 REQUIRED)

target_link_libraries(foe_bringup_lib PRIVATE glfw)

target_sources(
  foe_bringup_imgui
  PRIVATE imgui.cpp
)

target_sources(foe_bringup_lib PRIVATE window.cpp)