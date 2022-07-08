// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_HPP
#define MESH_HPP

struct foeMesh;
struct foeMeshCreateInfo;

void imgui_foeMesh(foeMesh const *pResource);

void imgui_foeMeshCreateInfo(foeMeshCreateInfo const *pCreateInfo);

#endif // MESH_HPP