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

#include <foe/position/yaml/import_registration.h>

#include <foe/imex/yaml/importer.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/yaml/exception.hpp>

#include "3d.hpp"
#include "error_code.hpp"

namespace {

bool importPosition3D(YAML::Node const &node,
                      foeEcsGroupTranslator groupTranslator,
                      foeEntityID entity,
                      foeSimulation const *pSimulation) {
    if (auto dataNode = node[yaml_position3d_key()]; dataNode) {
        auto *pPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

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

} // namespace

extern "C" foeErrorCode foePositionYamlRegisterImporters() {
    std::error_code errC;

    // Components
    if (!foeImexYamlRegisterComponentFn(yaml_position3d_key(), importPosition3D)) {
        errC = FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER;
        goto REGISTRATION_ERROR;
    }

REGISTRATION_ERROR:
    if (errC)
        foePositionYamlDeregisterImporters();

    return foeToErrorCode(errC);
}

extern "C" void foePositionYamlDeregisterImporters() {
    // Components
    foeImexYamlDeregisterComponentFn(yaml_position3d_key(), importPosition3D);
}