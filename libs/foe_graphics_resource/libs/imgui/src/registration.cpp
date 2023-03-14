// Copyright (C) 2021-2023 George Cave.
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

    foeResourceDecrementRefCount(resource);

    // foeImage
    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE) {
        imgui_foeResource(resource);

        if (ImGui::CollapsingHeader("Data")) {
            if (foeResourceGetState(resource) & FOE_RESOURCE_STATE_LOADED_BIT) {
                imgui_foeImage((foeImage const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResultSet result =
                foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &createInfo);
            if (result.value != FOE_SUCCESS || createInfo == FOE_NULL_HANDLE)
                // @TODO - Implement proper error handling
                std::abort();

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

        if (ImGui::CollapsingHeader("Data")) {
            if (foeResourceGetState(resource) & FOE_RESOURCE_STATE_LOADED_BIT) {
                imgui_foeMaterial((foeMaterial const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResultSet result =
                foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &createInfo);
            if (result.value != FOE_SUCCESS || createInfo == FOE_NULL_HANDLE)
                // @TODO - Implement proper error handling
                std::abort();

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

        if (ImGui::CollapsingHeader("Data")) {
            if (foeResourceGetState(resource) & FOE_RESOURCE_STATE_LOADED_BIT) {
                imgui_foeMesh((foeMesh const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResultSet result =
                foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &createInfo);
            if (result.value != FOE_SUCCESS || createInfo == FOE_NULL_HANDLE)
                // @TODO - Implement proper error handling
                std::abort();

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

        if (ImGui::CollapsingHeader("Data")) {
            if (foeResourceGetState(resource) & FOE_RESOURCE_STATE_LOADED_BIT) {
                imgui_foeShader((foeShader const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResultSet result =
                foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &createInfo);
            if (result.value != FOE_SUCCESS || createInfo == FOE_NULL_HANDLE)
                // @TODO - Implement proper error handling
                std::abort();

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

        if (ImGui::CollapsingHeader("Data")) {
            if (foeResourceGetState(resource) & FOE_RESOURCE_STATE_LOADED_BIT) {
                imgui_foeVertexDescriptor(
                    (foeVertexDescriptor const *)foeResourceGetData(resource));
            }
        }

        if (ImGui::CollapsingHeader("CreateInfo")) {
            foeResourceCreateInfo createInfo = FOE_NULL_HANDLE;
            foeResultSet result =
                foeSimulationGetResourceCreateInfo(pSimulation, resourceID, &createInfo);
            if (result.value != FOE_SUCCESS || createInfo == FOE_NULL_HANDLE)
                // @TODO - Implement proper error handling
                std::abort();

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
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO) {
        imgui_foeMeshFileCreateInfo(
            (foeMeshFileCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO) {
        imgui_foeMeshCubeCreateInfo(
            (foeMeshCubeCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
    }

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO) {
        imgui_foeMeshIcosphereCreateInfo(
            (foeMeshIcosphereCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
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

foeResultSet foeGraphicsResourceImGuiRegister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(nullptr, imgui_foeGraphicsResources,
                                        imgui_GraphicsResourceCreateInfo, nullptr);
}

foeResultSet foeGraphicsResourceImGuiDeregister(foeSimulationImGuiRegistrar *pRegistrar) {
    return pRegistrar->registerElements(nullptr, imgui_foeGraphicsResources,
                                        imgui_GraphicsResourceCreateInfo, nullptr);
}