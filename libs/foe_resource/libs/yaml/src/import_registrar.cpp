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
#include <foe/resource/mesh_loader.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <foe/resource/shader_loader.hpp>
#include <foe/resource/shader_pool.hpp>
#include <foe/resource/vertex_descriptor_loader.hpp>
#include <foe/resource/vertex_descriptor_pool.hpp>

#include "armature.hpp"
#include "error_code.hpp"
#include "image.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

std::error_code armatureCreateProcessing(foeResourceID resource,
                                         foeResourceCreateInfoBase *pCreateInfo,
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

std::error_code vertexDescriptorCreateProcessing(
    foeResourceID resource,
    foeResourceCreateInfoBase *pCreateInfo,
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

        pYamlImporter->deregisterResourceFns(yaml_vertex_descriptor_key(),
                                             yaml_read_vertex_descriptor,
                                             vertexDescriptorCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_shader_key(), yaml_read_shader,
                                             shaderCreateProcessing);

        pYamlImporter->deregisterResourceFns(yaml_image_key(), yaml_read_image,
                                             imageCreateProcessing);
    }
}

std::error_code onRegister(foeImporterGenerator *pGenerator) {
    std::error_code errC;

    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns(yaml_armature_key(), yaml_read_armature,
                                                armatureCreateProcessing)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        if (!pYamlImporter->registerResourceFns(yaml_mesh_key(), yaml_read_mesh,
                                                meshCreateProcessing)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_MESH_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        if (!pYamlImporter->registerResourceFns(yaml_vertex_descriptor_key(),
                                                yaml_read_vertex_descriptor,
                                                vertexDescriptorCreateProcessing)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_VERTEX_DESCRIPTOR_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        if (!pYamlImporter->registerResourceFns(yaml_shader_key(), yaml_read_shader,
                                                shaderCreateProcessing)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_SHADER_IMPORTER;
            goto REGISTRATION_FAILED;
        }

        if (!pYamlImporter->registerResourceFns(yaml_image_key(), yaml_read_image,
                                                imageCreateProcessing)) {
            errC = FOE_RESOURCE_YAML_ERROR_FAILED_TO_REGISTER_IMAGE_IMPORTER;
            goto REGISTRATION_FAILED;
        }
    }

REGISTRATION_FAILED:
    if (errC)
        onDeregister(pGenerator);

    return errC;
}

} // namespace

auto foeResourceRegisterYamlImportFunctionality() -> std::error_code {
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