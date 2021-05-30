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

#include "function_registrar.hpp"

#include <foe/physics/yaml/resource/collision_shape.hpp>
#include <foe/resource/yaml/armature.hpp>
#include <foe/resource/yaml/image.hpp>
#include <foe/resource/yaml/material.hpp>
#include <foe/resource/yaml/mesh.hpp>
#include <foe/resource/yaml/shader.hpp>
#include <foe/resource/yaml/vertex_descriptor.hpp>

#include <foe/imex/yaml/generator.hpp>

#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/yaml/component/rigid_body.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/position/yaml/component/3d.hpp>
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
#include <foe/yaml/exception.hpp>

#include "../armature_state.hpp"
#include "../camera.hpp"
#include "../camera_pool.hpp"
#include "../render_state.hpp"

struct foeResourceCreateInfoBase;
struct foeResourceLoaderBase;
struct foeResourcePoolBase;

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

bool collisionShapeCreateProcessing(foeResourceID resource,
                                    foeResourceCreateInfoBase *pCreateInfo,
                                    std::vector<foeResourceLoaderBase *> &resourceLoaders,
                                    std::vector<foeResourcePoolBase *> &resourcePools) {
    foePhysCollisionShapePool *pCollisionShapePool{nullptr};
    for (auto &it : resourcePools) {
        pCollisionShapePool = dynamic_cast<foePhysCollisionShapePool *>(it);

        if (pCollisionShapePool != nullptr)
            break;
    }

    foePhysCollisionShapeLoader *pCollisionShapeLoader{nullptr};
    for (auto &it : resourceLoaders) {
        pCollisionShapeLoader = dynamic_cast<foePhysCollisionShapeLoader *>(it);

        if (pCollisionShapeLoader != nullptr)
            break;
    }

    if (pCollisionShapePool == nullptr || pCollisionShapeLoader == nullptr)
        return false;

    auto *pCollisionShape = new foePhysCollisionShape{resource, pCollisionShapeLoader};

    if (!pCollisionShapePool->add(pCollisionShape)) {
        delete pCollisionShape;
        return false;
    }

    return true;
}

bool importArmature(YAML::Node const &node,
                    foeIdGroupTranslator const *pGroupTranslator,
                    foeEntityID entity,
                    std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node["armature_state"]; dataNode) {
        foeArmatureStatePool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foeArmatureStatePool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            foeArmatureState data = yaml_read_ArmatureState(dataNode, pGroupTranslator);

            pPool->insert(entity, std::move(data));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{"armature_state::" + e.whatStr()};
        }
    }

    return false;
}

bool importRenderState(YAML::Node const &node,
                       foeIdGroupTranslator const *pGroupTranslator,
                       foeEntityID entity,
                       std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node["render_state"]; dataNode) {
        foeRenderStatePool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foeRenderStatePool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            foeRenderState data = yaml_read_RenderState(dataNode, pGroupTranslator);

            pPool->insert(entity, std::move(data));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{"render_state::" + e.whatStr()};
        }
    }

    return false;
}

bool importPosition3D(YAML::Node const &node,
                      foeIdGroupTranslator const *,
                      foeEntityID entity,
                      std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node["position_3d"]; dataNode) {
        foePosition3dPool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foePosition3dPool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            std::unique_ptr<foePosition3d> pData(new foePosition3d);
            *pData = yaml_read_Position3D(dataNode);

            pPool->insert(entity, std::move(pData));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{"position_3d::" + e.whatStr()};
        }
    }

    return false;
}

bool importCamera(YAML::Node const &node,
                  foeIdGroupTranslator const *,
                  foeEntityID entity,
                  std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node["camera"]; dataNode) {
        foeCameraPool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foeCameraPool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            std::unique_ptr<Camera> pData(new Camera);
            *pData = yaml_read_Camera(dataNode);

            pPool->insert(entity, std::move(pData));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{"camera::" + e.whatStr()};
        }
    }

    return false;
}

bool importRigidBody(YAML::Node const &node,
                     foeIdGroupTranslator const *pGroupTranslator,
                     foeEntityID entity,
                     std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node["rigid_body"]; dataNode) {
        foeRigidBodyPool *pPool;

        for (auto it : componentPools) {
            pPool = dynamic_cast<foeRigidBodyPool *>(it);
            if (pPool != nullptr)
                break;
        }

        if (pPool == nullptr)
            return false;

        try {
            foeRigidBody data = yaml_read_RigidBody(dataNode, pGroupTranslator);

            pPool->insert(entity, std::move(data));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{"rigid_body::" + e.whatStr()};
        }
    }

    return false;
}

} // namespace

bool foeYamlCoreResourceFunctionRegistrar::registerFunctions(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resource
        if (!pYamlImporter->addImporter("armature_v1", yaml_read_armature_definition,
                                        armatureCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->addImporter("mesh_v1", yaml_read_mesh_definition, meshCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->addImporter("material_v1", yaml_read_material_definition,
                                        materialCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->addImporter("vertex_descriptor_v1",
                                        yaml_read_vertex_descriptor_definition,
                                        vertexDescriptorCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->addImporter("shader_v1", yaml_read_shader_definition,
                                        shaderCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->addImporter("image_v1", yaml_read_image_definition,
                                        imageCreateProcessing))
            goto FAILED_TO_ADD;

        if (!pYamlImporter->addImporter("collision_shape_v1", yaml_read_collision_shape_definition,
                                        collisionShapeCreateProcessing))
            goto FAILED_TO_ADD;

        // Component
        pYamlImporter->addComponentImporter("armature_state", importArmature);
        pYamlImporter->addComponentImporter("render_state", importRenderState);
        pYamlImporter->addComponentImporter("position_3d", importPosition3D);
        pYamlImporter->addComponentImporter("camera", importCamera);
        pYamlImporter->addComponentImporter("rigid_body", importRigidBody);
    }

    return true;

FAILED_TO_ADD:
    deregisterFunctions(pGenerator);
    return false;
}

bool foeYamlCoreResourceFunctionRegistrar::deregisterFunctions(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resource
        pYamlImporter->removeImporter("armature_v1", yaml_read_armature_definition,
                                      armatureCreateProcessing);

        pYamlImporter->removeImporter("mesh_v1", yaml_read_mesh_definition, meshCreateProcessing);

        pYamlImporter->removeImporter("material_v1", yaml_read_material_definition,
                                      materialCreateProcessing);

        pYamlImporter->removeImporter("vertex_descriptor_v1",
                                      yaml_read_vertex_descriptor_definition,
                                      vertexDescriptorCreateProcessing);

        pYamlImporter->removeImporter("shader_v1", yaml_read_shader_definition,
                                      shaderCreateProcessing);

        pYamlImporter->removeImporter("image_v1", yaml_read_image_definition,
                                      imageCreateProcessing);

        pYamlImporter->removeImporter("collision_shape_v1", yaml_read_collision_shape_definition,
                                      collisionShapeCreateProcessing);

        // Component
        pYamlImporter->removeComponentImporter("armature_state", importArmature);
        pYamlImporter->removeComponentImporter("render_state", importRenderState);
        pYamlImporter->removeComponentImporter("position_3d", importPosition3D);
        pYamlImporter->removeComponentImporter("camera", importCamera);
        pYamlImporter->removeComponentImporter("rigid_body", importRigidBody);
    }

    return true;
}