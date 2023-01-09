// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "position_3d.h"

#include <foe/position/binary.h>
#include <foe/position/component/3d.hpp>
#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet export_foePosition3D(foeEntityID entity,
                                             foeSimulation const *pSimulation,
                                             foeImexBinarySet *pBinarySets) {
    foeResultSet result = to_foeResult(FOE_POSITION_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    foePosition3dPool componentPool = (foePosition3dPool)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (componentPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(componentPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(componentPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;

            foePosition3d **ppPositionData =
                (foePosition3d **)foeEcsComponentPoolDataPtr(componentPool) + offset;

            result = binary_write_foePosition3d(*ppPositionData, &set.dataSize, nullptr);
            if (result.value != FOE_SUCCESS)
                goto EXPORT_FAILED;

            set.pData = malloc(set.dataSize);
            if (set.pData == NULL) {
                result = to_foeResult(FOE_POSITION_BINARY_ERROR_OUT_OF_MEMORY);
                goto EXPORT_FAILED;
            }

            result = binary_write_foePosition3d(*ppPositionData, &set.dataSize, set.pData);
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
    foePosition3dPool componentPool = (foePosition3dPool)foeSimulationGetComponentPool(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
    if (componentPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_POSITION_BINARY_ERROR_POSITION_3D_POOL_NOT_FOUND);

    std::unique_ptr<foePosition3d> pComponentData(new foePosition3d);
    foePosition3d *pData = pComponentData.get();
    foeResultSet result = binary_read_foePosition3d(pReadBuffer, pReadSize, pComponentData.get());
    if (result.value == FOE_SUCCESS)
        result = foeEcsComponentPoolInsert(componentPool, entity, &pData);

    if (result.value == FOE_SUCCESS)
        pComponentData.release();

    return result;
}