# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(foe_imgui_vk PRIVATE error_code.c render_graph_job_imgui.cpp
                                    renderer.cpp vk_result.c)
