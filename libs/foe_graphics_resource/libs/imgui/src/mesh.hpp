// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MESH_HPP
#define MESH_HPP

struct foeMesh;
struct foeMeshFileCreateInfo;
struct foeMeshCubeCreateInfo;
struct foeMeshIcosphereCreateInfo;

void imgui_foeMesh(foeMesh const *pResource);

void imgui_foeMeshFileCreateInfo(foeMeshFileCreateInfo const *pCreateInfo);

void imgui_foeMeshCubeCreateInfo(foeMeshCubeCreateInfo const *pCreateInfo);

void imgui_foeMeshIcosphereCreateInfo(foeMeshIcosphereCreateInfo const *pCreateInfo);

#endif // MESH_HPP