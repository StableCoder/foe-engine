# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

find_package(glfw3 REQUIRED)

target_link_libraries(foe_skunkworks_lib PUBLIC glfw)

target_sources(foe_skunkworks_lib PRIVATE window.cpp)

target_link_libraries(foe_skunkworks_imgui PUBLIC glfw)

target_sources(foe_skunkworks_imgui PRIVATE imgui.cpp)
