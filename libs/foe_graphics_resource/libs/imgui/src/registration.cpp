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

#include <foe/graphics/resource/imgui/registration.hpp>

#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/simulation/imgui/registrar.hpp>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

void imgui_foeGraphicsResources(foeResourceID resource,
                                foeResourcePoolBase **ppPools,
                                size_t poolCount) {
    for (size_t i = 0; i < poolCount; ++i) {
        { // foeImage
            auto *pPool = dynamic_cast<foeImagePool *>(ppPools[i]);
            if (pPool != nullptr) {
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeImage(pResource);
                }
            }
        }

        { // foeMaterial
            auto *pPool = dynamic_cast<foeMaterialPool *>(ppPools[i]);
            if (pPool != nullptr) {
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeMaterial(pResource);
                }
            }
        }

        { // foeMesh
            auto *pPool = dynamic_cast<foeMeshPool *>(ppPools[i]);
            if (pPool != nullptr) {
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeMesh(pResource);
                }
            }
        }

        { // foeShader
            auto *pPool = dynamic_cast<foeShaderPool *>(ppPools[i]);
            if (pPool != nullptr) {
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeShader(pResource);
                }
            }
        }

        { // foeVertexDescriptor
            auto *pPool = dynamic_cast<foeVertexDescriptorPool *>(ppPools[i]);
            if (pPool != nullptr) {
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeVertexDescriptor(pResource);
                }
            }
        }
    }
}

} // namespace

auto foeGraphicsResourceImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(nullptr, &imgui_foeGraphicsResources, nullptr);
}

auto foeGraphicsResourceImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar)
    -> std::error_code {
    return pRegistrar->registerElements(nullptr, &imgui_foeGraphicsResources, nullptr);
}