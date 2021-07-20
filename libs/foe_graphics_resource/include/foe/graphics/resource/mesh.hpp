/*
    Copyright (C) 2021 George Cave.

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
#include <foe/graphics/resource/export.h>
#include <foe/model/armature.hpp>
#include <foe/model/vertex_component.hpp>
#include <foe/simulation/core/resource.hpp>

#include <vector>

struct FOE_GFX_RES_EXPORT foeMesh : public foeResourceBase {
    foeMesh(foeResourceID resource, foeResourceFns const *pResourceFns);
    ~foeMesh();

    void loadCreateInfo();
    void loadResource(bool refreshCreateInfo);
    void unloadResource();

    struct Data {
        void *pUnloadContext{nullptr};
        void (*pUnloadFn)(void *, void *, uint32_t, bool){nullptr};
        std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;

        foeGfxMesh gfxData{};
        std::vector<foeMeshBone> gfxBones{};
        std::vector<foeVertexComponent> gfxVertexComponent{};
        uint32_t perVertexBoneWeights{0};
    };
    Data data{};
};

#endif // FOE_GRAPHICS_RESOURCE_MESH_HPP