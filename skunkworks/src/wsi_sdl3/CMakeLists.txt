# Copyright (C) 2025 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

find_package(SDL3 REQUIRED COMPONENTS SDL3)

target_link_libraries(foe_skunkworks_lib PUBLIC SDL3::SDL3)

target_sources(foe_skunkworks_lib PRIVATE window.cpp)

target_link_libraries(foe_skunkworks_imgui PUBLIC SDL3::SDL3)

target_sources(foe_skunkworks_imgui PRIVATE imgui.cpp)
