// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/imgui/registration.hpp>

#include <foe/graphics/resource/type_defs.h>
#include <foe/simulation/imgui/registrar.hpp>
#include <foe/simulation/simulation.hpp>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

void imgui_foeGraphicsResources(foeResourceBase const *pResourceData) {
    if (pResourceData->rType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE)
        imgui_foeImage((foeImage const *)pResourceData);

    if (pResourceData->rType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL)
        imgui_foeMaterial((foeMaterial const *)pResourceData);

    if (pResourceData->rType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH)
        imgui_foeMesh((foeMesh const *)pResourceData);

    if (pResourceData->rType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER)
        imgui_foeShader((foeShader const *)pResourceData);

    if (pResourceData->rType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR)
        imgui_foeVertexDescriptor((foeVertexDescriptor const *)pResourceData);
}

void imgui_GraphicsResourceCreateInfo(foeResourceCreateInfo resourceCreateInfo) {
    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO)
        imgui_foeImageCreateInfo(
            (foeImageCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO)
        imgui_foeMaterialCreateInfo(
            (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_FILE_CREATE_INFO)
        imgui_foeMeshFileCreateInfo(
            (foeMeshFileCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CUBE_CREATE_INFO)
        imgui_foeMeshCubeCreateInfo(
            (foeMeshCubeCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_ICOSPHERE_CREATE_INFO)
        imgui_foeMeshIcosphereCreateInfo(
            (foeMeshIcosphereCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO)
        imgui_foeShaderCreateInfo(
            (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));

    if (foeResourceCreateInfoGetType(resourceCreateInfo) ==
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO)
        imgui_foeVertexDescriptorCreateInfo((
            foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(resourceCreateInfo));
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