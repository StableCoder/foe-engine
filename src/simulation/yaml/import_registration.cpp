// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "import_registration.h"

#include <foe/imex/yaml/importer.hpp>
#include <foe/resource/pool.h>
#include <foe/simulation/simulation.hpp>
#include <foe/yaml/exception.hpp>

#include "../armature.hpp"
#include "../armature_state_imex.hpp"
#include "../armature_state_pool.hpp"
#include "../render_state_imex.hpp"
#include "../render_state_pool.h"
#include "../type_defs.h"
#include "armature.hpp"
#include "result.h"

namespace {

// Resources

foeResultSet armatureCreateProcessing(foeResourceID resourceID,
                                      foeResourceCreateInfo createInfo,
                                      foeSimulation const *pSimulation) {
    foeResource armature =
        foeResourcePoolAdd(pSimulation->resourcePool, resourceID,
                           FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE, sizeof(foeArmature));

    if (armature == FOE_NULL_HANDLE)
        return to_foeResult(FOE_BRINGUP_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS);

    return to_foeResult(FOE_BRINGUP_YAML_SUCCESS);
}

// Components

bool importArmatureState(YAML::Node const &node,
                         foeEcsGroupTranslator groupTranslator,
                         foeEntityID entity,
                         foeSimulation const *pSimulation) {
    if (auto dataNode = node[yaml_armature_state_key()]; dataNode) {
        foeArmatureStatePool armatureStatePool =
            (foeArmatureStatePool)foeSimulationGetComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

        if (armatureStatePool == FOE_NULL_HANDLE)
            return false;

        try {
            foeArmatureState data = yaml_read_ArmatureState(dataNode, groupTranslator);

            foeResultSet result = foeEcsComponentPoolInsert(armatureStatePool, entity, &data);
            if (result.value != FOE_SUCCESS) {
                return false;
            }

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_armature_state_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

bool importRenderState(YAML::Node const &node,
                       foeEcsGroupTranslator groupTranslator,
                       foeEntityID entity,
                       foeSimulation const *pSimulation) {
    if (auto dataNode = node[yaml_render_state_key()]; dataNode) {
        foeRenderStatePool renderStatePool = (foeRenderStatePool)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);

        if (renderStatePool == FOE_NULL_HANDLE)
            return false;

        try {
            foeRenderState data = yaml_read_RenderState(dataNode, groupTranslator);

            foeEcsComponentPoolInsert(renderStatePool, entity, &data);

            return true;
        } catch (foeYamlException const &e) {
            throw foeYamlException{std::string{yaml_render_state_key()} + "::" + e.whatStr()};
        }
    }

    return false;
}

} // namespace

extern "C" foeResultSet foeBringupYamlRegisterImporters() {
    foeResultSet result = to_foeResult(FOE_BRINGUP_YAML_SUCCESS);

    // Resources
    if (!foeImexYamlRegisterResourceFns(yaml_armature_key(), yaml_read_armature,
                                        armatureCreateProcessing)) {
        result = to_foeResult(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    // Component
    if (!foeImexYamlRegisterComponentFn(yaml_armature_state_key(), importArmatureState)) {
        result = to_foeResult(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER);
        goto REGISTRATION_FAILED;
    }

    if (!foeImexYamlRegisterComponentFn(yaml_render_state_key(), importRenderState)) {
        result = to_foeResult(FOE_BRINGUP_YAML_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER);
        goto REGISTRATION_FAILED;
    }

REGISTRATION_FAILED:
    if (result.value != FOE_SUCCESS)
        foeBringupYamlDeregisterImporters();

    return result;
}

extern "C" void foeBringupYamlDeregisterImporters() {
    // Component
    foeImexYamlDeregisterComponentFn(yaml_armature_state_key(), importArmatureState);
    foeImexYamlDeregisterComponentFn(yaml_render_state_key(), importRenderState);

    // Resources
    foeImexYamlDeregisterResourceFns(yaml_armature_key(), yaml_read_armature,
                                     armatureCreateProcessing);
}
