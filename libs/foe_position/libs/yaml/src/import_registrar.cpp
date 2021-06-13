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

#include <foe/position/yaml/import_registrar.hpp>

#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/generator.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/yaml/exception.hpp>

#include "3d.hpp"

namespace {

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

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources

        // Component
        pYamlImporter->removeComponentImporter("position_3d", importPosition3D);
    }
}

void onRegister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Resources

        // Component
        pYamlImporter->addComponentImporter("position_3d", importPosition3D);
    }

    return;

FAILED_TO_ADD:
    onDeregister(pGenerator);
}

} // namespace

void foePositionRegisterYamlImportFunctionality() {
    foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foePositionDeregisterYamlImportFunctionality() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}