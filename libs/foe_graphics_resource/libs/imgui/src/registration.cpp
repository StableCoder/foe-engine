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
#include <foe/resource/imgui/create_info.h>
#include <foe/resource/imgui/resource.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>
#include <imgui.h>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

void imgui_foeGraphicsResources(
    foeResourceID resource,
    foeSimulation const *pSimulation,
    std::function<void(foeResourceCreateInfo)> showResourceCreateInfoDataFn) {
    // foeImage
    auto *pImagePool = (foeImagePool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

    if (pImagePool != nullptr) {
        foeResource res = pImagePool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE) {
            imgui_foeResource(res);

            std::string resDataHeader = "Data: ";
            resDataHeader += foeResourceLoadStateToString(foeResourceGetState(res));
            if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
                if (foeResourceGetState(res) == foeResourceLoadState::Loaded) {
                    imgui_foeImage((foeImage const *)foeResourceGetData(res));
                }
            }

            if (ImGui::CollapsingHeader("CreateInfo")) {
                foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(res);
                if (createInfo != FOE_NULL_HANDLE) {
                    imgui_foeResourceCreateInfo(createInfo);
                    ImGui::Separator();
                    showResourceCreateInfoDataFn(createInfo);

                    foeResourceCreateInfoDecrementRefCount(createInfo);
                }
            }
        }
    }

    // foeMaterial
    auto pMaterialPool = (foeMaterialPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL);

    if (pMaterialPool != nullptr) {
        foeResource res = pMaterialPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL) {
            imgui_foeResource(res);

            std::string resDataHeader = "Data: ";
            resDataHeader += foeResourceLoadStateToString(foeResourceGetState(res));
            if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
                if (foeResourceGetState(res) == foeResourceLoadState::Loaded) {
                    imgui_foeMaterial((foeMaterial const *)foeResourceGetData(res));
                }
            }

            if (ImGui::CollapsingHeader("CreateInfo")) {
                foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(res);
                if (createInfo != FOE_NULL_HANDLE) {
                    imgui_foeResourceCreateInfo(createInfo);
                    ImGui::Separator();
                    showResourceCreateInfoDataFn(createInfo);

                    foeResourceCreateInfoDecrementRefCount(createInfo);
                }
            }
        }
    }

    // foeMesh
    auto *pMeshPool = (foeMeshPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

    if (pMeshPool != nullptr) {
        foeResource res = pMeshPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH) {
            imgui_foeResource(res);

            std::string resDataHeader = "Data: ";
            resDataHeader += foeResourceLoadStateToString(foeResourceGetState(res));
            if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
                if (foeResourceGetState(res) == foeResourceLoadState::Loaded) {
                    imgui_foeMesh((foeMesh const *)foeResourceGetData(res));
                }
            }

            if (ImGui::CollapsingHeader("CreateInfo")) {
                foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(res);
                if (createInfo != FOE_NULL_HANDLE) {
                    imgui_foeResourceCreateInfo(createInfo);
                    ImGui::Separator();
                    showResourceCreateInfoDataFn(createInfo);

                    foeResourceCreateInfoDecrementRefCount(createInfo);
                }
            }
        }
    }

    // foeShader
    auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

    if (pShaderPool != nullptr) {
        foeResource res = pShaderPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER) {
            imgui_foeResource(res);

            std::string resDataHeader = "Data: ";
            resDataHeader += foeResourceLoadStateToString(foeResourceGetState(res));
            if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
                if (foeResourceGetState(res) == foeResourceLoadState::Loaded) {
                    imgui_foeShader((foeShader const *)foeResourceGetData(res));
                }
            }

            if (ImGui::CollapsingHeader("CreateInfo")) {
                foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(res);
                if (createInfo != FOE_NULL_HANDLE) {
                    imgui_foeResourceCreateInfo(createInfo);
                    ImGui::Separator();
                    showResourceCreateInfoDataFn(createInfo);

                    foeResourceCreateInfoDecrementRefCount(createInfo);
                }
            }
        }
    }

    // foeVertexDescriptor
    auto *pVertexDescriptorPool = (foeVertexDescriptorPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL);

    if (pVertexDescriptorPool != nullptr) {
        foeResource res = pVertexDescriptorPool->find(resource);
        if (res != FOE_NULL_HANDLE &&
            foeResourceGetType(res) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) {
            imgui_foeResource(res);

            std::string resDataHeader = "Data: ";
            resDataHeader += foeResourceLoadStateToString(foeResourceGetState(res));
            if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
                if (foeResourceGetState(res) == foeResourceLoadState::Loaded) {
                    imgui_foeVertexDescriptor((foeVertexDescriptor const *)foeResourceGetData(res));
                }
            }

            if (ImGui::CollapsingHeader("CreateInfo")) {
                foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(res);
                if (createInfo != FOE_NULL_HANDLE) {
                    imgui_foeResourceCreateInfo(createInfo);
                    ImGui::Separator();
                    showResourceCreateInfoDataFn(createInfo);

                    foeResourceCreateInfoDecrementRefCount(createInfo);
                }
            }
        }
    }
}

void imgui_GraphicsResourceCreateInfo(foeResourceCreateInfo resourceCreateInfo) {
    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO) {
        imgui_foeImageCreateInfo(
            (foeImageCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO) {
        imgui_foeMaterialCreateInfo(
            (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CREATE_INFO) {
        imgui_foeMeshCreateInfo(
            (foeMeshCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO) {
        imgui_foeShaderCreateInfo(
            (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO) {
        imgui_foeVertexDescriptorCreateInfo((
            foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }
}

} // namespace

auto foeGraphicsResourceImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) -> std::error_code {
    return pRegistrar->registerElements(nullptr, imgui_foeGraphicsResources,
                                        imgui_GraphicsResourceCreateInfo, nullptr);
}

auto foeGraphicsResourceImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar)
    -> std::error_code {
    return pRegistrar->registerElements(nullptr, imgui_foeGraphicsResources,
                                        imgui_GraphicsResourceCreateInfo, nullptr);
}