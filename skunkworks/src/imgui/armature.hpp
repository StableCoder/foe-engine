// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMGUI_ARMATURE_HPP
#define IMGUI_ARMATURE_HPP

struct foeArmature;
struct foeArmatureCreateInfo;

void imgui_foeArmature(foeArmature const *pResource);

void imgui_foeArmatureCreateInfo(foeArmatureCreateInfo const *pCreateInfo);

#endif // IMGUI_ARMATURE_HPP