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

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/material.hpp>
#include <foe/graphics/resource/mesh.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor.hpp>
#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>

#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "result.h"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

foeResult imageCreateProcessing(foeResourceID resourceID,
                                foeResourceCreateInfo createInfo,
                                foeSimulation const *pSimulation) {
    foeResource image =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE, sizeof(foeImage));

    if (image == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);
}

foeResult materialCreateProcessing(foeResourceID resourceID,
                                   foeResourceCreateInfo createInfo,
                                   foeSimulation const *pSimulation) {
    foeResource material =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL, sizeof(foeMaterial));

    if (material == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);
}

foeResult meshCreateProcessing(foeResourceID resourceID,
                               foeResourceCreateInfo createInfo,
                               foeSimulation const *pSimulation) {
    foeResource mesh =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH, sizeof(foeMesh));

    if (mesh == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);
}

foeResult shaderCreateProcessing(foeResourceID resourceID,
                                 foeResourceCreateInfo createInfo,
                                 foeSimulation const *pSimulation) {
    auto *pShader =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, sizeof(foeShader));

    if (!pShader)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);
}

foeResult vertexDescriptorCreateProcessing(foeResourceID resourceID,
                                           foeResourceCreateInfo createInfo,
                                           foeSimulation const *pSimulation) {
    auto *pVertexResource = foeResourcePoolAdd(
        pSimulation->resourcePool, resourceID,
        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR, sizeof(foeVertexDescriptor));

    if (!pVertexResource)
        return to_foeResult(
            FOE_GRAPHICS_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);
}

} // namespace

extern "C" foeResult foeGraphicsResourceYamlRegisterImporters() {
    foeResult result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_SUCCESS);

    // Resources
    if (!foeImexYamlRegisterResourceFns(yaml_image_key(), yaml_read_image, imageCreateProcessing)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER);
        goto REGISTRATION_FAILED;
    }
    if (!foeImexYamlRegisterResourceFns(yaml_material_key(), yaml_read_material,
                                        materialCreateProcessing)) {
        result =
            to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MATERIAL_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_mesh_key(), yaml_read_mesh, meshCreateProcessing)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterResourceFns(yaml_shader_key(), yaml_read_shader,
                                        shaderCreateProcessing)) {
        result = to_foeResult(FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER);
        goto REGISTRATION_FAILED;
    }
    if (!foeImexYamlRegisterResourceFns(yaml_vertex_descriptor_key(), yaml_read_vertex_descriptor,
                                        vertexDescriptorCreateProcessing)) {
        result = to_foeResult(
            FOE_GRAPHICS_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER);
        goto REGISTRATION_FAILED;
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeGraphicsResourceYamlDeregisterImporters();

    return result;
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