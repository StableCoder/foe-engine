// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "position_3d.h"

#include <foe/position/binary.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet export_foePosition3D(foeEntityID entity,
                                             foeSimulation const *pSimulation,
                                             foeImexBinarySet *pBinarySets) {
    foeResultSet result = to_foeResult(FOE_POSITION_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    auto *pComponentPool = (foePosition3dPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (pComponentPool) {
        if (auto offset = pComponentPool->find(entity); offset != pComponentPool->size()) {
            auto *pComponentData = pComponentPool->begin<1>() + offset;

            result = binary_write_foePosition3d(pComponentData->get(), &set.dataSize, nullptr);
            if (result.value != FOE_SUCCESS)
                goto EXPORT_FAILED;

            set.pData = malloc(set.dataSize);
            if (set.pData == NULL) {
                result = to_foeResult(FOE_POSITION_BINARY_ERROR_OUT_OF_MEMORY);
                goto EXPORT_FAILED;
            }

            result = binary_write_foePosition3d(pComponentData->get(), &set.dataSize, set.pData);
            set.pKey = binary_key_foePosition3d();
        }
    }

EXPORT_FAILED:
    if (result.value != FOE_SUCCESS && set.pData)
        free(set.pData);
    if (result.value == FOE_SUCCESS)
        *pBinarySets = set;

    return result;
}

extern "C" foeResultSet import_foePosition3D(void const *pReadBuffer,
                                             uint32_t *pReadSize,
                                             foeEcsGroupTranslator groupTranslator,
                                             foeEntityID entity,
                                             foeSimulation const *pSimulation) {
    auto *pComponentPool = (foePosition3dPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (pComponentPool == nullptr)
        return to_foeResult(FOE_POSITION_BINARY_ERROR_POSITION_3D_POOL_NOT_FOUND);

    std::unique_ptr<foePosition3d> pComponentData(new foePosition3d);
    foeResultSet result = binary_read_foePosition3d(pReadBuffer, pReadSize, pComponentData.get());
    if (result.value == FOE_SUCCESS)
        pComponentPool->insert(entity, std::move(pComponentData));

    return result;
}