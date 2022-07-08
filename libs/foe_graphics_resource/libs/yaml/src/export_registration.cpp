// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/yaml/export_registration.h>

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/mesh_loader.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/imex/exporters.hpp>
#include <foe/imex/yaml/exporter.hpp>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "result.h"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::vector<foeKeyYamlPair> exportResource(foeResourceID resourceID,
                                           foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResource resource = foeResourcePoolFind(pSimulation->resourcePool, resourceID);

    if (resource == FOE_NULL_HANDLE)
        return keyDataPairs;

    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE) {
        auto createInfo = foeResourceGetCreateInfo(resource);
        if (foeResourceCreateInfoGetType(createInfo) ==
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_CREATE_INFO) {
            auto const *pCreateInfo =
                (foeImageCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_image_key(),
                .data = yaml_write_image(*pCreateInfo),
            });
        }
    }

    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL) {
        auto createInfo = foeResourceGetCreateInfo(resource);
        if (foeResourceCreateInfoGetType(createInfo) ==
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO) {
            auto const *pCreateInfo =
                (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
            auto const *pMaterial = (foeMaterial const *)foeResourceGetData(resource);

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_material_key(),
                .data = yaml_write_material(*pCreateInfo, pMaterial->pGfxFragDescriptor),
            });
        }
    }

    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH) {
        auto createInfo = foeResourceGetCreateInfo(resource);
        if (foeResourceCreateInfoGetType(createInfo) ==
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_CREATE_INFO) {
            auto const *pCreateInfo =
                (foeMeshCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_mesh_key(),
                .data = yaml_write_mesh(*pCreateInfo),
            });
        }
    }

    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER) {
        auto createInfo = foeResourceGetCreateInfo(resource);
        if (foeResourceCreateInfoGetType(createInfo) ==
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO) {
            auto const *pCreateInfo =
                (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_shader_key(),
                .data = yaml_write_shader(*pCreateInfo),
            });
        }
    }

    if (foeResourceGetType(resource) == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) {
        auto createInfo = foeResourceGetCreateInfo(resource);
        if (foeResourceCreateInfoGetType(createInfo) ==
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO) {
            auto const *pCreateInfo =
                (foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

            keyDataPairs.emplace_back(foeKeyYamlPair{
                .key = yaml_vertex_descriptor_key(),
                .data = yaml_write_vertex_descriptor(*pCreateInfo),
            });
        }
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        foeImexYamlDeregisterResourceFn(exportResource);
    }
}

foeResult onRegister(foeExporter exporter) {
    foeResult result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        result = foeImexYamlRegisterResourceFn(exportResource);
        if (result.value != FOE_SUCCESS) {
            result =
                to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTER);
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        onDeregister(exporter);

    return result;
}

} // namespace

extern "C" foeResult foeGraphicsResourceYamlRegisterExporters() {
    return foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

extern "C" void foeGraphicsResourceYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}