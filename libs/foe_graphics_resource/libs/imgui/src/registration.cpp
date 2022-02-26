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
#include <foe/graphics/resource/type_defs.h>
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

            if (ppPools[i]->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL) {
                auto *pPool = (foeImagePool *)ppPools[i];
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeImage(pResource);
                }
            }
        }

        { // foeMaterial
            if (ppPools[i]->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL) {
                auto *pPool = (foeMaterialPool *)ppPools[i];
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeMaterial(pResource);
                }
            }
        }

        { // foeMesh
            if (ppPools[i]->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL) {
                auto *pPool = (foeMeshPool *)ppPools[i];
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeMesh(pResource);
                }
            }
        }

        { // foeShader
            if (ppPools[i]->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL) {
                auto *pPool = (foeShaderPool *)ppPools[i];
                auto pResource = pPool->find(resource);
                if (pResource != nullptr) {
                    imgui_foeShader(pResource);
                }
            }
        }

        { // foeVertexDescriptor

            if (ppPools[i]->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL) {
                auto *pPool = (foeVertexDescriptorPool *)ppPools[i];
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