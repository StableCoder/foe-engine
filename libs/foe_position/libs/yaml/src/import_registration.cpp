// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/yaml/import_registration.h>

#include <foe/imex/yaml/importer.hpp>
#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
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
        foePosition3dPool componentPool = (foePosition3dPool)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        if (componentPool == FOE_NULL_HANDLE)
            return false;

        try {
            std::unique_ptr<foePosition3d> pData(new foePosition3d);
            *pData = yaml_read_position3d(dataNode);

            foePosition3d *ppData = pData.get();

            foeResultSet result = foeEcsComponentPoolInsert(componentPool, entity, &ppData);
            if (result.value == FOE_SUCCESS) {
                pData.release();
                return true;
            } else {
                return false;
            }
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_position3d_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

} // namespace

extern "C" foeResultSet foePositionYamlRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_POSITION_YAML_SUCCESS);

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