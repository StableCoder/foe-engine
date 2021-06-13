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

#include "import_registrar.hpp"

#include <foe/imex/yaml/generator.hpp>
#include <foe/yaml/exception.hpp>

#include "armature_state_imex.hpp"
#include "armature_state_pool.hpp"
#include "camera_imex.hpp"
#include "camera_pool.hpp"
#include "render_state_imex.hpp"
#include "render_state_pool.hpp"

namespace {

bool importArmatureState(YAML::Node const &node,
                         foeIdGroupTranslator const *pGroupTranslator,
                         foeEntityID entity,
                         std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node[yaml_armature_state_key()]; dataNode) {
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
            throw foeYamlException{std::string{yaml_armature_state_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

bool importRenderState(YAML::Node const &node,
                       foeIdGroupTranslator const *pGroupTranslator,
                       foeEntityID entity,
                       std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node[yaml_render_state_key()]; dataNode) {
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
            throw foeYamlException{std::string{yaml_render_state_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

bool importCamera(YAML::Node const &node,
                  foeIdGroupTranslator const *,
                  foeEntityID entity,
                  std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node[yaml_camera_key()]; dataNode) {
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

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources

        // Component
        pYamlImporter->deregisterComponentFn(yaml_armature_state_key(), importArmatureState);
        pYamlImporter->deregisterComponentFn(yaml_render_state_key(), importRenderState);
        pYamlImporter->deregisterComponentFn(yaml_camera_key(), importCamera);
    }
}

void onRegister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources

        // Component
        pYamlImporter->registerComponentFn(yaml_armature_state_key(), importArmatureState);
        pYamlImporter->registerComponentFn(yaml_render_state_key(), importRenderState);
        pYamlImporter->registerComponentFn(yaml_camera_key(), importCamera);
    }

    return;

FAILED_TO_ADD:
    onDeregister(pGenerator);
}

} // namespace

void foeBringupRegisterYamlImportFunctionality() {
    foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foeBringupDeregisterYamlImportFunctionality() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}