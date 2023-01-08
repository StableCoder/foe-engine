// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_state.h"

#include <foe/simulation/simulation.hpp>

#include "../binary.h"
#include "../render_state.h"
#include "../render_state_pool.h"
#include "result.h"

extern "C" foeResultSet export_foeRenderState(foeEntityID entity,
                                              foeSimulation const *pSimulation,
                                              foeImexBinarySet *pBinarySet) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    foeRenderStatePool componentPool = (foeRenderStatePool)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
    if (componentPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;
            foeRenderState *pComponentData =
                (foeRenderState *)foeEcsComponentPoolDataPtr(componentPool) + offset;

            result = binary_write_foeRenderState(pComponentData, &set.dataSize, nullptr);
            if (result.value != FOE_SUCCESS)
                goto COMPONENT_EXPORT_FAILED;

            set.pData = malloc(set.dataSize);
            if (set.pData == NULL) {
                result = to_foeResult(FOE_BRINGUP_BINARY_ERROR_OUT_OF_MEMORY);
                goto COMPONENT_EXPORT_FAILED;
            }

            result = binary_write_foeRenderState(pComponentData, &set.dataSize, set.pData);
            set.pKey = binary_key_foeRenderState();
        }
    }

COMPONENT_EXPORT_FAILED:
    if (result.value != FOE_SUCCESS && set.pData)
        free(set.pData);
    if (result.value == FOE_SUCCESS)
        *pBinarySet = set;

    return result;
}

extern "C" foeResultSet import_foeRenderState(void const *pReadBuffer,
                                              uint32_t *pReadSize,
                                              foeEcsGroupTranslator groupTranslator,
                                              foeEntityID entity,
                                              foeSimulation const *pSimulation) {
    foeRenderStatePool componentPool = (foeRenderStatePool)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
    if (componentPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_BRINGUP_BINARY_ERROR_RENDER_STATE_POOL_NOT_FOUND);

    foeRenderState componentData;
    foeResultSet result =
        binary_read_foeRenderState(pReadBuffer, pReadSize, groupTranslator, &componentData);
    if (result.value == FOE_SUCCESS)
        result = foeEcsComponentPoolInsert(componentPool, entity, &componentData);

    return result;
}