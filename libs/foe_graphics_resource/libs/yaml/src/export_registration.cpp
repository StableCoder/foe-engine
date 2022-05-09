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

#include "error_code.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::vector<foeKeyYamlPair> exportImage(foeResourceID resourceID,
                                        foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResourcePool imagePool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

    if (imagePool != FOE_NULL_HANDLE) {
        foeResource image = foeResourcePoolFind(imagePool, resourceID);

        if (image != FOE_NULL_HANDLE) {
            auto createInfo = foeResourceGetCreateInfo(image);
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
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportMaterial(foeResourceID resourceID,
                                           foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResourcePool materialPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL);

    if (materialPool != FOE_NULL_HANDLE) {
        foeResource material = foeResourcePoolFind(materialPool, resourceID);

        if (material != FOE_NULL_HANDLE) {
            auto createInfo = foeResourceGetCreateInfo(material);
            if (foeResourceCreateInfoGetType(createInfo) ==
                FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO) {
                auto const *pCreateInfo =
                    (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
                auto const *pMaterial = (foeMaterial const *)foeResourceGetData(material);

                keyDataPairs.emplace_back(foeKeyYamlPair{
                    .key = yaml_material_key(),
                    .data = yaml_write_material(*pCreateInfo, pMaterial->pGfxFragDescriptor),
                });
            }
        }
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportMesh(foeResourceID resourceID, foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResourcePool meshPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

    if (meshPool != FOE_NULL_HANDLE) {
        foeResource mesh = foeResourcePoolFind(meshPool, resourceID);
        if (mesh != FOE_NULL_HANDLE) {
            auto createInfo = foeResourceGetCreateInfo(mesh);
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
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportShader(foeResourceID resourceID,
                                         foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResourcePool shaderPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

    if (shaderPool != FOE_NULL_HANDLE) {
        foeResource shader = foeResourcePoolFind(shaderPool, resourceID);
        if (shader != FOE_NULL_HANDLE) {
            auto createInfo = foeResourceGetCreateInfo(shader);
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
    }

    return keyDataPairs;
}

std::vector<foeKeyYamlPair> exportVertexDescriptor(foeResourceID resourceID,
                                                   foeSimulation const *pSimulation) {
    std::vector<foeKeyYamlPair> keyDataPairs;

    foeResourcePool vertexDescriptorPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL);

    if (vertexDescriptorPool != FOE_NULL_HANDLE) {
        foeResource vertexDescriptor = foeResourcePoolFind(vertexDescriptorPool, resourceID);

        if (vertexDescriptor != FOE_NULL_HANDLE) {
            auto createInfo = foeResourceGetCreateInfo(vertexDescriptor);
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
    }

    return keyDataPairs;
}

void onDeregister(foeExporter exporter) {
    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        foeImexYamlDeregisterResourceFn(exportVertexDescriptor);
        foeImexYamlDeregisterResourceFn(exportShader);
        foeImexYamlDeregisterResourceFn(exportMesh);
        foeImexYamlDeregisterResourceFn(exportMaterial);
        foeImexYamlDeregisterResourceFn(exportImage);
    }
}

std::error_code onRegister(foeExporter exporter) {
    std::error_code errC;

    if (std::string_view{exporter.pName} == "Yaml") {
        // Resource
        if (foeImexYamlRegisterResourceFn(exportImage)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportMaterial)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportMesh)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportShader)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_EXPORTER;
            goto REGISTRATION_FAILED;
        }
        if (foeImexYamlRegisterResourceFn(exportVertexDescriptor)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_EXPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(exporter);

    return errC;
}

} // namespace

extern "C" foeErrorCode foeGraphicsResourceYamlRegisterExporters() {
    auto errC = foeRegisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });

    return foeToErrorCode(errC);
}

extern "C" void foeGraphicsResourceYamlDeregisterExporters() {
    foeDeregisterExportFunctionality(foeExportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}