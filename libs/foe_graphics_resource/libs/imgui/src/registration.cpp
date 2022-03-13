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

#include <foe/graphics/resource/imgui/registration.hpp>

#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

void imgui_foeGraphicsResources(foeResourceID resource, foeSimulation const *pSimulation) {
    // foeImage
    auto *pImagePool = (foeImagePool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

    if (pImagePool != nullptr) {
        foeResource res = pImagePool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE) {
            imgui_foeImage(res);
        }
    }

    // foeMaterial
    auto pMaterialPool = (foeMaterialPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL);

    if (pMaterialPool != nullptr) {
        foeResource res = pMaterialPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL) {
            imgui_foeMaterial(res);
        }
    }

    // foeMesh
    auto *pMeshPool = (foeMeshPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

    if (pMeshPool != nullptr) {
        foeResource res = pMeshPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH) {
            imgui_foeMesh(res);
        }
    }

    // foeShader
    auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

    if (pShaderPool != nullptr) {
        foeResource res = pShaderPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER) {
            imgui_foeShader(res);
        }
    }

    // foeVertexDescriptor
    auto *pVertexDescriptorPool = (foeVertexDescriptorPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL);

    if (pVertexDescriptorPool != nullptr) {
        foeResource res = pVertexDescriptorPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) {
            imgui_foeVertexDescriptor(res);
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