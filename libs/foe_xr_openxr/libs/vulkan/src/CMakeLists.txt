# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(
  foe_xr_openxr_vk PRIVATE error_code.c log.cpp render_graph_jobs_swapchain.cpp
                           vk_result.c vulkan.cpp xr_result.c)
