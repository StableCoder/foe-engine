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

#include <foe/imex/yaml/generator.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/position/yaml/component/3d.hpp>
#include <foe/yaml/exception.hpp>

#include "../armature_state.hpp"
#include "../camera.hpp"
#include "../camera_pool.hpp"
#include "../render_state.hpp"

struct foeResourceCreateInfoBase;
struct foeResourceLoaderBase;
struct foeResourcePoolBase;

namespace {

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

} // namespace

bool foeYamlCoreResourceFunctionRegistrar::registerFunctions(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources

        // Component
        pYamlImporter->addComponentImporter("armature_state", importArmature);
        pYamlImporter->addComponentImporter("render_state", importRenderState);
        pYamlImporter->addComponentImporter("position_3d", importPosition3D);
        pYamlImporter->addComponentImporter("camera", importCamera);
    }

    return true;

FAILED_TO_ADD:
    deregisterFunctions(pGenerator);
    return false;
}

bool foeYamlCoreResourceFunctionRegistrar::deregisterFunctions(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources

        // Component
        pYamlImporter->removeComponentImporter("armature_state", importArmature);
        pYamlImporter->removeComponentImporter("render_state", importRenderState);
        pYamlImporter->removeComponentImporter("position_3d", importPosition3D);
        pYamlImporter->removeComponentImporter("camera", importCamera);
    }

    return true;
}