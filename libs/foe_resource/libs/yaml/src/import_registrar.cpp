/*
    Copyright (C) 2021 George Cave.

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

#include <foe/resource/yaml/import_registrar.hpp>

#include <foe/imex/yaml/generator.hpp>
#include <foe/resource/armature_loader.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/image_loader.hpp>
#include <foe/resource/image_pool.hpp>
#include <foe/resource/material_loader.hpp>
#include <foe/resource/material_pool.hpp>
#include <foe/resource/mesh_loader.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <foe/resource/shader_loader.hpp>
#include <foe/resource/shader_pool.hpp>
#include <foe/resource/vertex_descriptor_loader.hpp>
#include <foe/resource/vertex_descriptor_pool.hpp>

#include "armature.hpp"
#include "error_code.hpp"
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::error_code armatureCreateProcessing(foeResourceID resource,
                                         foeResourceCreateInfoBase *pCreateInfo,
                                         std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                         std::vector<foeResourcePoolBase *> &resourcePools) {
    foeArmaturePool *pArmaturePool{nullptr};
    for (auto &it : resourcePools) {
        pArmaturePool = dynamic_cast<foeArmaturePool *>(it);

        if (pArmaturePool != nullptr)
            break;
    }

    if (pArmaturePool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_ARMATURE_POOL_NOT_FOUND;

    auto *pArmature = pArmaturePool->add(resource);

    if (!pArmature)
        return FOE_RESOURCE_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

std::error_code meshCreateProcessing(foeResourceID resource,
                                     foeResourceCreateInfoBase *pCreateInfo,
                                     std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                     std::vector<foeResourcePoolBase *> &resourcePools) {
    foeMeshPool *pMeshPool{nullptr};
    for (auto &it : resourcePools) {
        pMeshPool = dynamic_cast<foeMeshPool *>(it);

        if (pMeshPool != nullptr)
            break;
    }

    if (pMeshPool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_MESH_POOL_NOT_FOUND;

    auto *pMesh = pMeshPool->add(resource);

    if (!pMesh)
        return FOE_RESOURCE_YAML_ERROR_MESH_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

std::error_code materialCreateProcessing(foeResourceID resource,
                                         foeResourceCreateInfoBase *pCreateInfo,
                                         std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                         std::vector<foeResourcePoolBase *> &resourcePools) {
    foeMaterialPool *pMaterialPool{nullptr};
    for (auto &it : resourcePools) {
        pMaterialPool = dynamic_cast<foeMaterialPool *>(it);

        if (pMaterialPool != nullptr)
            break;
    }

    if (pMaterialPool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_MATERIAL_POOL_NOT_FOUND;

    auto *pMaterial = pMaterialPool->add(resource);

    if (!pMaterial)
        return FOE_RESOURCE_YAML_ERROR_MATERIAL_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

std::error_code vertexDescriptorCreateProcessing(
    foeResourceID resource,
    foeResourceCreateInfoBase *pCreateInfo,
    std::vector<foeResourceLoaderBase *> &resourceLoaders,
    std::vector<foeResourcePoolBase *> &resourcePools) {
    foeVertexDescriptorPool *pVertexDescriptorPool{nullptr};
    for (auto &it : resourcePools) {
        pVertexDescriptorPool = dynamic_cast<foeVertexDescriptorPool *>(it);

        if (pVertexDescriptorPool != nullptr)
            break;
    }

    if (pVertexDescriptorPool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_POOL_NOT_FOUND;

    auto *pVertexDescriptor = pVertexDescriptorPool->add(resource);

    if (!pVertexDescriptor)
        return FOE_RESOURCE_YAML_ERROR_VERTEX_DESCRIPTOR_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

std::error_code shaderCreateProcessing(foeResourceID resource,
                                       foeResourceCreateInfoBase *pCreateInfo,
                                       std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                       std::vector<foeResourcePoolBase *> &resourcePools) {
    foeShaderPool *pShaderPool{nullptr};
    for (auto &it : resourcePools) {
        pShaderPool = dynamic_cast<foeShaderPool *>(it);

        if (pShaderPool != nullptr)
            break;
    }

    if (pShaderPool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_SHADER_POOL_NOT_FOUND;

    auto *pShader = pShaderPool->add(resource);
    if (!pShader)
        return FOE_RESOURCE_YAML_ERROR_SHADER_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

std::error_code imageCreateProcessing(foeResourceID resource,
                                      foeResourceCreateInfoBase *pCreateInfo,
                                      std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                      std::vector<foeResourcePoolBase *> &resourcePools) {
    foeImagePool *pImagePool{nullptr};
    for (auto &it : resourcePools) {
        pImagePool = dynamic_cast<foeImagePool *>(it);

        if (pImagePool != nullptr)
            break;
    }

    if (pImagePool == nullptr)
        return FOE_RESOURCE_YAML_ERROR_IMAGE_POOL_NOT_FOUND;

    auto *pImage = pImagePool->add(resource);
    if (!pImage)
        return FOE_RESOURCE_YAML_ERROR_IMAGE_RESOURCE_ALREADY_EXISTS;

    return FOE_RESOURCE_YAML_SUCCESS;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        pYamlImporter->deregisterResourceFns(yaml_armature_key(), yaml_read_armature,
                                             armatureCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_mesh_key(), yaml_read_mesh, meshCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_material_key(), yaml_read_material,
                                             materialCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_vertex_descriptor_key(),
                                             yaml_read_vertex_descriptor,
                                             vertexDescriptorCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_shader_key(), yaml_read_shader,
                                             shaderCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_image_key(), yaml_read_image,
                                             imageCreateProcessing);

        // Components
    }
}

void onRegister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns(yaml_armature_key(), yaml_read_armature,
                                                armatureCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns(yaml_mesh_key(), yaml_read_mesh,
                                                meshCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns(yaml_material_key(), yaml_read_material,
                                                materialCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns(yaml_vertex_descriptor_key(),
                                                yaml_read_vertex_descriptor,
                                                vertexDescriptorCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns(yaml_shader_key(), yaml_read_shader,
                                                shaderCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns(yaml_image_key(), yaml_read_image,
                                                imageCreateProcessing))
            goto FAILED_TO_ADD;

        // Components
    }

    return;

FAILED_TO_ADD:
    onDeregister(pGenerator);
}

} // namespace

bool foeResourceRegisterYamlImportFunctionality() {
    return foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeResourceDeregisterYamlImportFunctionality() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}