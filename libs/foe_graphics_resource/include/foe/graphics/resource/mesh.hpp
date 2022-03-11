/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef FOE_GRAPHICS_RESOURCE_MESH_HPP
#define FOE_GRAPHICS_RESOURCE_MESH_HPP

#include <foe/graphics/mesh.hpp>
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