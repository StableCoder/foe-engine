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

#include <foe/position/yaml/import_registration.hpp>

#include <foe/imex/importers.hpp>
#include <foe/imex/yaml/generator.hpp>
#include <foe/imex/yaml/importer.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/yaml/exception.hpp>

#include "3d.hpp"
#include "error_code.hpp"

namespace {

bool importPosition3D(YAML::Node const &node,
                      foeIdGroupTranslator const *,
                      foeEntityID entity,
                      std::vector<foeComponentPoolBase *> &componentPools) {
    if (auto dataNode = node[yaml_position3d_key()]; dataNode) {
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
            *pData = yaml_read_position3d(dataNode);

            pPool->insert(entity, std::move(pData));

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_position3d_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

void onDeregister(foeImporterGenerator *pGenerator) {
    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Components
        pYamlImporter->deregisterComponentFn(yaml_position3d_key(), importPosition3D);
    }
}

std::error_code onRegister(foeImporterGenerator *pGenerator) {
    std::error_code errC;

    if (auto pYamlImporter = dynamic_cast<foeYamlImporterGenerator *>(pGenerator); pYamlImporter) {
        // Components
        if (!pYamlImporter->registerComponentFn(yaml_position3d_key(), importPosition3D)) {
            errC = FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER;
            goto REGISTRATION_ERROR;
        }
    }

REGISTRATION_ERROR:
    if (errC)
        onDeregister(pGenerator);

    return errC;
}

} // namespace

auto foePositionYamlRegisterImporters() -> std::error_code {
    return foeRegisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}

void foePositionYamlDeregisterImporters() {
    foeDeregisterImportFunctionality(foeImportFunctionality{
        .onRegister = onRegister,
        .onDeregister = onDeregister,
    });
}