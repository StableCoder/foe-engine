// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "render_state.h"

#include <foe/simulation/simulation.hpp>

#include "../binary.h"
#include "../render_state.hpp"
#include "../render_state_pool.hpp"
#include "result.h"

extern "C" foeResultSet export_foeRenderState(foeEntityID entity,
                                              foeSimulation const *pSimulation,
                                              foeImexBinarySet *pBinarySet) {
    foeResultSet result = to_foeResult(FOE_BRINGUP_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    auto *pComponentPool = (foeRenderStatePool *)foeSimulationGetComponentPool(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
    if (pComponentPool) {
        if (auto searchIt = pComponentPool->find(entity); searchIt != pComponentPool->size()) {
            auto *pComponentData = pComponentPool->begin<1>() + searchIt;

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