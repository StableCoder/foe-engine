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

#include <foe/graphics/resource/yaml/import_registration.h>

#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/generator.hpp>
#include <foe/imex/yaml/importer.hpp>

#include "error_code.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::error_code imageCreateProcessing(foeResourceID resource,
                                      foeResourceCreateInfo createInfo,
                                      foeSimulation const *pSimulation) {
    auto *pImagePool = (foeImagePool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

    if (pImagePool == nullptr)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND;

    auto *pImage = pImagePool->add(resource);

    if (!pImage)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code materialCreateProcessing(foeResourceID resource,
                                         foeResourceCreateInfo createInfo,
                                         foeSimulation const *pSimulation) {
    auto *pMaterialPool = (foeMaterialPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL);

    if (pMaterialPool == nullptr)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND;

    auto *pMaterial = pMaterialPool->add(resource);

    if (!pMaterial)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code meshCreateProcessing(foeResourceID resource,
                                     foeResourceCreateInfo createInfo,
                                     foeSimulation const *pSimulation) {
    auto *pMeshPool = (foeMeshPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

    if (pMeshPool == nullptr)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND;

    auto *pMesh = pMeshPool->add(resource);

    if (!pMesh)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code shaderCreateProcessing(foeResourceID resource,
                                       foeResourceCreateInfo createInfo,
                                       foeSimulation const *pSimulation) {
    auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

    if (pShaderPool == nullptr)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND;

    auto *pShader = pShaderPool->add(resource);

    if (!pShader)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code vertexDescriptorCreateProcessing(foeResourceID resource,
                                                 foeResourceCreateInfo createInfo,
                                                 foeSimulation const *pSimulation) {
    auto *pVertexDescriptorPool = (foeVertexDescriptorPool *)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL);

    if (pVertexDescriptorPool == nullptr)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND;

    auto *pVertexResource = pVertexDescriptorPool->add(resource);

    if (!pVertexResource)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        pYamlImporter->deregisterResourceFns(yaml_vertex_descriptor_key(),
                                             yaml_read_vertex_descriptor,
                                             vertexDescriptorCreateProcessing);
        pYamlImporter->deregisterResourceFns(yaml_shader_key(), yaml_read_shader,
                                             shaderCreateProcessing);
        pYamlImporter->deregisterResourceFns(yaml_mesh_key(), yaml_read_mesh, meshCreateProcessing);
        pYamlImporter->deregisterResourceFns(yaml_material_key(), yaml_read_material,
                                             materialCreateProcessing);
        pYamlImporter->deregisterResourceFns(yaml_image_key(), yaml_read_image,
                                             imageCreateProcessing);
    }
}

std::error_code onRegister(foeImporterGenerator *pGenerator) {
    std::error_code errC;

    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns(yaml_image_key(), yaml_read_image,
                                                imageCreateProcessing)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER;
            goto REGISTRATION_FAILED;
        }
        if (!pYamlImporter->registerResourceFns(yaml_material_key(), yaml_read_material,
                                                materialCreateProcessing)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        if (!pYamlImporter->registerResourceFns(yaml_mesh_key(), yaml_read_mesh,
                                                meshCreateProcessing)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        if (!pYamlImporter->registerResourceFns(yaml_shader_key(), yaml_read_shader,
                                                shaderCreateProcessing)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER;
            goto REGISTRATION_FAILED;
        }
        if (!pYamlImporter->registerResourceFns(yaml_vertex_descriptor_key(),
                                                yaml_read_vertex_descriptor,
                                                vertexDescriptorCreateProcessing)) {
            errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pGenerator);

    return foeToErrorCode(errC);
}

} // namespace

extern "C" foeErrorCode foeGraphicsResourceYamlRegisterImporters() {
    std::error_code errC = foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });

    return foeToErrorCode(errC);
}

extern "C" void foeGraphicsResourceYamlDeregisterImporters() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}