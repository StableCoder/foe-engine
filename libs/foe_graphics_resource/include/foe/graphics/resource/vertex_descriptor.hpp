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

#ifndef FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP
#define FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP

#include <foe/graphics/resource/export.h>
#include <foe/graphics/vk/vertex_descriptor.hpp>
#include <foe/simulation/core/resource.hpp>

struct foeShader;
struct foeGfxVertexDescriptor;

struct FOE_GFX_RES_EXPORT foeVertexDescriptor : public foeResourceBase {
    foeVertexDescriptor(foeResourceID resource, foeResourceFns const *pResourceFns);
    ~foeVertexDescriptor();

    void loadCreateInfo();
    void loadResource(bool refreshCreateInfo);
    void unloadResource();

    struct Data {
        void *pUnloadContext;
        void (*pUnloadFn)(void *, void *, uint32_t, bool);
        std::shared_ptr<foeResourceCreateInfoBase> pCreateInfo;

        foeShader *pVertex;
        foeShader *pTessellationControl;
        foeShader *pTessellationEvaluation;
        foeShader *pGeometry;

        foeGfxVertexDescriptor vertexDescriptor;
    };
    Data data{};
};

#endif // FOE_GRAPHICS_RESOURCE_VERTEX_DESCRIPTOR_HPP