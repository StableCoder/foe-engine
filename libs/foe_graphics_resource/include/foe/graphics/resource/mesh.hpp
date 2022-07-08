// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_GRAPHICS_RESOURCE_MESH_HPP
#define FOE_GRAPHICS_RESOURCE_MESH_HPP

#include <foe/graphics/mesh.h>
#include <foe/model/armature.hpp>
#include <foe/model/vertex_component.hpp>

#include <vector>

struct foeMesh {
    foeGfxMesh gfxData{};
    std::vector<foeMeshBone> gfxBones{};
    std::vector<foeVertexComponent> gfxVertexComponent{};
    uint32_t perVertexBoneWeights{0};
};

#endif // FOE_GRAPHICS_RESOURCE_MESH_HPP