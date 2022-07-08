# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(foe_wsi_glfw3 PRIVATE error_code.c keyboard.cpp mouse.cpp
                                     vk_result.c vulkan.cpp window.cpp)
