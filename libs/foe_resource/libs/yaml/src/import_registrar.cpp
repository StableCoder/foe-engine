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
#include "image.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"
#include "vertex_descriptor.hpp"

namespace {

bool armatureCreateProcessing(foeResourceID resource,
                              foeResourceCreateInfoBase *pCreateInfo,
                              std::vector<foeResourceLoaderBase *> &resourceLoaders,
                              std::vector<foeResourcePoolBase *> &resourcePools) {
    foeArmaturePool *pArmaturePool{nullptr};
    for (auto &it : resourcePools) {
        pArmaturePool = dynamic_cast<foeArmaturePool *>(it);

        if (pArmaturePool != nullptr)
            break;
    }

    foeArmatureLoader *pArmatureLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pArmatureLoader = dynamic_cast<foeArmatureLoader *>(it);

        if (pArmatureLoader != nullptr)
            break;
    }

    if (pArmaturePool == nullptr || pArmatureLoader == nullptr)
        return false;

    auto *pArmature = new foeArmature{resource, pArmatureLoader};

    if (!pArmaturePool->add(pArmature)) {
        delete pArmature;
        return false;
    }

    return true;
}

bool meshCreateProcessing(foeResourceID resource,
                          foeResourceCreateInfoBase *pCreateInfo,
                          std::vector<foeResourceLoaderBase *> &resourceLoaders,
                          std::vector<foeResourcePoolBase *> &resourcePools) {
    foeMeshPool *pMeshPool{nullptr};
    for (auto &it : resourcePools) {
        pMeshPool = dynamic_cast<foeMeshPool *>(it);

        if (pMeshPool != nullptr)
            break;
    }

    foeMeshLoader *pMeshLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pMeshLoader = dynamic_cast<foeMeshLoader *>(it);

        if (pMeshLoader != nullptr)
            break;
    }

    if (pMeshPool == nullptr || pMeshLoader == nullptr)
        return false;

    auto *pMesh = new foeMesh{resource, pMeshLoader};

    if (!pMeshPool->add(pMesh)) {
        delete pMesh;
        return false;
    }

    return true;
}

bool materialCreateProcessing(foeResourceID resource,
                              foeResourceCreateInfoBase *pCreateInfo,
                              std::vector<foeResourceLoaderBase *> &resourceLoaders,
                              std::vector<foeResourcePoolBase *> &resourcePools) {
    foeMaterialPool *pMaterialPool{nullptr};
    for (auto &it : resourcePools) {
        pMaterialPool = dynamic_cast<foeMaterialPool *>(it);

        if (pMaterialPool != nullptr)
            break;
    }

    foeMaterialLoader *pMaterialLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pMaterialLoader = dynamic_cast<foeMaterialLoader *>(it);

        if (pMaterialLoader != nullptr)
            break;
    }

    if (pMaterialPool == nullptr || pMaterialLoader == nullptr)
        return false;

    auto *pMaterial = new foeMaterial{resource, pMaterialLoader};

    if (!pMaterialPool->add(pMaterial)) {
        delete pMaterial;
        return false;
    }

    return true;
}

bool vertexDescriptorCreateProcessing(foeResourceID resource,
                                      foeResourceCreateInfoBase *pCreateInfo,
                                      std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                      std::vector<foeResourcePoolBase *> &resourcePools) {
    foeVertexDescriptorPool *pVertexDescriptorPool{nullptr};
    for (auto &it : resourcePools) {
        pVertexDescriptorPool = dynamic_cast<foeVertexDescriptorPool *>(it);

        if (pVertexDescriptorPool != nullptr)
            break;
    }

    foeVertexDescriptorLoader *pVertexDescriptorLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pVertexDescriptorLoader = dynamic_cast<foeVertexDescriptorLoader *>(it);

        if (pVertexDescriptorLoader != nullptr)
            break;
    }

    if (pVertexDescriptorPool == nullptr || pVertexDescriptorLoader == nullptr)
        return false;

    auto *pVertexDescriptor = new foeVertexDescriptor{resource, pVertexDescriptorLoader};

    if (!pVertexDescriptorPool->add(pVertexDescriptor)) {
        delete pVertexDescriptor;
        return false;
    }

    return true;
}

bool shaderCreateProcessing(foeResourceID resource,
                            foeResourceCreateInfoBase *pCreateInfo,
                            std::vector<foeResourceLoaderBase *> &resourceLoaders,
                            std::vector<foeResourcePoolBase *> &resourcePools) {
    foeShaderPool *pShaderPool{nullptr};
    for (auto &it : resourcePools) {
        pShaderPool = dynamic_cast<foeShaderPool *>(it);

        if (pShaderPool != nullptr)
            break;
    }

    foeShaderLoader *pShaderLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pShaderLoader = dynamic_cast<foeShaderLoader *>(it);

        if (pShaderLoader != nullptr)
            break;
    }

    if (pShaderPool == nullptr || pShaderLoader == nullptr)
        return false;

    auto *pShader = new foeShader{resource, pShaderLoader};

    if (!pShaderPool->add(pShader)) {
        delete pShader;
        return false;
    }

    return true;
}

bool imageCreateProcessing(foeResourceID resource,
                           foeResourceCreateInfoBase *pCreateInfo,
                           std::vector<foeResourceLoaderBase *> &resourceLoaders,
                           std::vector<foeResourcePoolBase *> &resourcePools) {
    foeImagePool *pImagePool{nullptr};
    for (auto &it : resourcePools) {
        pImagePool = dynamic_cast<foeImagePool *>(it);

        if (pImagePool != nullptr)
            break;
    }

    foeImageLoader *pImageLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pImageLoader = dynamic_cast<foeImageLoader *>(it);

        if (pImageLoader != nullptr)
            break;
    }

    if (pImagePool == nullptr || pImageLoader == nullptr)
        return false;

    auto *pImage = new foeImage{resource, pImageLoader};

    if (!pImagePool->add(pImage)) {
        delete pImage;
        return false;
    }

    return true;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        pYamlImporter->deregisterResourceFns("armature_v1", yaml_read_armature_definition,
                                      armatureCreateProcessing);

        pYamlImporter->deregisterResourceFns("mesh_v1", yaml_read_mesh_definition, meshCreateProcessing);

        pYamlImporter->deregisterResourceFns("material_v1", yaml_read_material_definition,
                                      materialCreateProcessing);

        pYamlImporter->deregisterResourceFns("vertex_descriptor_v1",
                                      yaml_read_vertex_descriptor_definition,
                                      vertexDescriptorCreateProcessing);

        pYamlImporter->deregisterResourceFns("shader_v1", yaml_read_shader_definition,
                                      shaderCreateProcessing);

        pYamlImporter->deregisterResourceFns("image_v1", yaml_read_image_definition,
                                      imageCreateProcessing);

        // Components
    }
}

void onRegister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources
        if (!pYamlImporter->registerResourceFns("armature_v1", yaml_read_armature_definition,
                                        armatureCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns("mesh_v1", yaml_read_mesh_definition, meshCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns("material_v1", yaml_read_material_definition,
                                        materialCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns("vertex_descriptor_v1",
                                        yaml_read_vertex_descriptor_definition,
                                        vertexDescriptorCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns("shader_v1", yaml_read_shader_definition,
                                        shaderCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->registerResourceFns("image_v1", yaml_read_image_definition,
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