# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

#!/usr/bin/env sh

glslangValidator -V -x -o imgui.vert.u32 imgui.vert
glslangValidator -V -x -o imgui.frag.u32 imgui.frag
