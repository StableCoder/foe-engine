// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_state.h"

#include <foe/simulation/simulation.hpp>

#include "../armature.hpp"
#include "../armature_state.h"
#include "../armature_state_pool.h"
#include "../binary.h"
#include "result.h"

extern "C" foeResultSet export_foeArmatureState(foeEntityID entity,
                                                foeSimulation const *pSimulation,
                                                foeImexBinarySet *pBinarySet) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    foeArmatureStatePool componentPool = (foeArmatureStatePool)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
    if (componentPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            foeArmatureState *pComponentData =
                (foeArmatureState *)foeEcsComponentPoolDataPtr(componentPool) + offset;

            result = binary_write_foeArmatureState(pComponentData, &set.dataSize, nullptr);
            if (result.value != FOE_SUCCESS)
                goto COMPONENT_EXPORT_FAILED;

            set.pData = malloc(set.dataSize);
            if (set.pData == NULL) {
                result = to_foeResult(FOE_BRINGUP_BINARY_ERROR_OUT_OF_MEMORY);
                goto COMPONENT_EXPORT_FAILED;
            }

            result = binary_write_foeArmatureState(pComponentData, &set.dataSize, set.pData);
            set.pKey = binary_key_foeArmatureState();
        }
    }

COMPONENT_EXPORT_FAILED:
    if (result.value != FOE_SUCCESS && set.pData)
        free(set.pData);
    if (result.value == FOE_SUCCESS)
        *pBinarySet = set;

    return result;
}

extern "C" foeResultSet import_foeArmatureState(void const *pReadBuffer,
                                                uint32_t *pReadSize,
                                                foeEcsGroupTranslator groupTranslator,
                                                foeEntityID entity,
                                                foeSimulation const *pSimulation) {
    foeArmatureStatePool componentPool = (foeArmatureStatePool)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
    if (componentPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_BRINGUP_BINARY_ERROR_ARMATURE_STATE_POOL_NOT_FOUND);

    foeArmatureState componentData;
    foeResultSet result =
        binary_read_foeArmatureState(pReadBuffer, pReadSize, groupTranslator, &componentData);
    if (result.value == FOE_SUCCESS) {
        result = foeEcsComponentPoolInsert(componentPool, entity, &componentData);
        if (result.value != FOE_SUCCESS) {
            return result;
        }
    }

    return result;
}