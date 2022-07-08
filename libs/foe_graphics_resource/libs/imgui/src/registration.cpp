// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/imgui/registration.hpp>

#include <foe/graphics/resource/type_defs.h>
#include <foe/resource/imgui/create_info.h>
#include <foe/resource/imgui/resource.h>
#include <foe/resource/pool.h>
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
    foeResourceID resourceID,
    foeSimulation const *pSimulation,
    std::function<void(foeResourceCreateInfo)> showResourceCreateInfoDataFn) {
    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);

    if (resource == FOE_NULL_HANDLE)
        return;

    // foeImage
    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
                imgui_foeImage((foeImage const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource);
            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        }
    }

    // foeMaterial
    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
                imgui_foeMaterial((foeMaterial const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource);
            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        }
    }

    // foeMesh
    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
                imgui_foeMesh((foeMesh const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource);
            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        }
    }

    // foeShader
    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
                imgui_foeShader((foeShader const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource);
            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        }
    }

    // foeVertexDescriptor
    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) {
        imgui_foeResource(resource);

        std::string resDataHeader = "Data: ";
        resDataHeader += foeResourceLoadStateToString(foeResourceGetState(resource));
        if (ImGui::CollapsingHeader(resDataHeader.c_str())) {
            if (foeResourceGetState(resource) == foeResourceLoadState::Loaded) {
                imgui_foeVertexDescriptor(
                    (foeVertexDescriptor const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = foeResourceGetCreateInfo(resource);
            if (createInfo != FOE_NULL_HANDLE) {
                imgui_foeResourceCreateInfo(createInfo);
                ImGui::Separator();
                showResourceCreateInfoDataFn(createInfo);

                foeResourceCreateInfoDecrementRefCount(createInfo);
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

foeResult foeGraphicsResourceImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(nullptr, imgui_foeGraphicsResources,
                                        imgui_GraphicsResourceCreateInfo, nullptr);
}

foeResult foeGraphicsResourceImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(nullptr, imgui_foeGraphicsResources,
                                        imgui_GraphicsResourceCreateInfo, nullptr);
}