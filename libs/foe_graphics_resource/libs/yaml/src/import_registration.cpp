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

#include <foe/graphics/resource/type_defs.h>
#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/resource/pool.h>

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
    foeResourcePool imagePool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

    if (imagePool == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND;

    foeResource image = foeResourcePoolAdd(imagePool, resource);

    if (image == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code materialCreateProcessing(foeResourceID resource,
                                         foeResourceCreateInfo createInfo,
                                         foeSimulation const *pSimulation) {
    foeResourcePool materialPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL);

    if (materialPool == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND;

    foeResource material = foeResourcePoolAdd(materialPool, resource);

    if (material == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code meshCreateProcessing(foeResourceID resource,
                                     foeResourceCreateInfo createInfo,
                                     foeSimulation const *pSimulation) {
    foeResourcePool meshPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

    if (meshPool == nullptr)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND;

    foeResource mesh = foeResourcePoolAdd(meshPool, resource);

    if (mesh == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code shaderCreateProcessing(foeResourceID resource,
                                       foeResourceCreateInfo createInfo,
                                       foeSimulation const *pSimulation) {
    foeResourcePool shaderPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

    if (shaderPool == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND;

    auto *pShader = foeResourcePoolAdd(shaderPool, resource);

    if (!pShader)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

std::error_code vertexDescriptorCreateProcessing(foeResourceID resource,
                                                 foeResourceCreateInfo createInfo,
                                                 foeSimulation const *pSimulation) {
    foeResourcePool vertexDescriptorPool = (foeResourcePool)foeSimulationGetResourcePool(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL);

    if (vertexDescriptorPool == FOE_NULL_HANDLE)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND;

    auto *pVertexResource = foeResourcePoolAdd(vertexDescriptorPool, resource);

    if (!pVertexResource)
        return FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS;

    return FOE_GRAPHICS_RESOURCE_YAML_SUCCESS;
}

} // namespace

extern "C" foeErrorCode foeGraphicsResourceYamlRegisterImporters() {
    std::error_code errC;

    // Resources
    if (!foeImexYamlRegisterResourceFns(yaml_image_key(), yaml_read_image, imageCreateProcessing)) {
        errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER;
        goto REGISTRATION_FAILED;
    }
    if (!foeImexYamlRegisterResourceFns(yaml_material_key(), yaml_read_material,
                                        materialCreateProcessing)) {
        errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER;
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_mesh_key(), yaml_read_mesh, meshCreateProcessing)) {
        errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER;
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_shader_key(), yaml_read_shader,
                                        shaderCreateProcessing)) {
        errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER;
        goto REGISTRATION_FAILED;
    }
    if (!foeImexYamlRegisterResourceFns(yaml_vertex_descriptor_key(), yaml_read_vertex_descriptor,
                                        vertexDescriptorCreateProcessing)) {
        errC = FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER;
        goto REGISTRATION_FAILED;
    }

REGISTRATION_FAILED:
    if (errC)
        foeGraphicsResourceYamlDeregisterImporters();

    return foeToErrorCode(errC);
}

extern "C" void foeGraphicsResourceYamlDeregisterImporters() {
    // Resources
    foeImexYamlDeregisterResourceFns(yaml_vertex_descriptor_key(), yaml_read_vertex_descriptor,
                                     vertexDescriptorCreateProcessing);
    foeImexYamlDeregisterResourceFns(yaml_shader_key(), yaml_read_shader, shaderCreateProcessing);
    foeImexYamlDeregisterResourceFns(yaml_mesh_key(), yaml_read_mesh, meshCreateProcessing);
    foeImexYamlDeregisterResourceFns(yaml_material_key(), yaml_read_material,
                                     materialCreateProcessing);
    foeImexYamlDeregisterResourceFns(yaml_image_key(), yaml_read_image, imageCreateProcessing);
}