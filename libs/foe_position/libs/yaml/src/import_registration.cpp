// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/yaml/import_registration.h>

#include <foe/imex/yaml/importer.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/type_defs.h>
#include <foe/yaml/exception.hpp>

#include "3d.hpp"
#include "result.h"

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

extern "C" foeResult foePositionYamlRegisterImporters() {
    foeResult result = to_foeResult(FOE_POSITION_YAML_SUCCESS);

    // Components
    if (!foeImexYamlRegisterComponentFn(yaml_position3d_key(), importPosition3D)) {
        result = to_foeResult(FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER);
        goto REGISTRATION_ERROR;
    }

REGISTRATION_ERROR:
    if (result.value != FOE_SUCCESS)
        foePositionYamlDeregisterImporters();

    return result;
}

extern "C" void foePositionYamlDeregisterImporters() {
    // Components
    foeImexYamlDeregisterComponentFn(yaml_position3d_key(), importPosition3D);
}