// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MATERIAL_HPP
#define MATERIAL_HPP

struct foeMaterial;
struct foeMaterialCreateInfo;

void imgui_foeMaterial(foeMaterial const *pResource);

void imgui_foeMaterialCreateInfo(foeMaterialCreateInfo const *pCreateInfo);

#endif // MATERIAL_HPP