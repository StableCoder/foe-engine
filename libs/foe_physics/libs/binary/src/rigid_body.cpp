// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "rigid_body.h"

#include <foe/physics/binary.h>
#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/type_defs.h>
#include <foe/simulation/simulation.hpp>

#include "result.h"

#include <algorithm>

extern "C" foeResultSet export_foeRigidBody(foeEntityID entity,
                                            foeSimulation const *pSimulation,
                                            foeImexBinarySet *pBinarySets) {
    foeResultSet result = to_foeResult(FOE_PHYSICS_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    foeRigidBodyPool rigidBodyPool = (foeRigidBodyPool)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
    if (rigidBodyPool != FOE_NULL_HANDLE) {
        foeEntityID const *const pStartID = foeEcsComponentPoolIdPtr(rigidBodyPool);
        foeEntityID const *const pEndID = pStartID + foeEcsComponentPoolSize(rigidBodyPool);

        foeEntityID const *pID = std::lower_bound(pStartID, pEndID, entity);

        if (pID != pEndID && *pID == entity) {
            size_t offset = pID - pStartID;

            foeRigidBody *pRigidBodyData =
                (foeRigidBody *)foeEcsComponentPoolDataPtr(rigidBodyPool) + offset;

            result = binary_write_foeRigidBody(pRigidBodyData, &set.dataSize, nullptr);
            if (result.value != FOE_SUCCESS)
                goto EXPORT_FAILED;

            set.pData = malloc(set.dataSize);
            if (set.pData == NULL) {
                result = to_foeResult(FOE_PHYSICS_BINARY_ERROR_OUT_OF_MEMORY);
                goto EXPORT_FAILED;
            }

            result = binary_write_foeRigidBody(pRigidBodyData, &set.dataSize, set.pData);
            set.pKey = binary_key_foeRigidBody();
        }
    }

EXPORT_FAILED:

    if (result.value != FOE_SUCCESS && set.pData)
        free(set.pData);
    if (result.value == FOE_SUCCESS)
        *pBinarySets = set;

    return result;
}

extern "C" foeResultSet import_foeRigidBody(void const *pReadBuffer,
                                            uint32_t *pReadSize,
                                            foeEcsGroupTranslator groupTranslator,
                                            foeEntityID entity,
                                            foeSimulation const *pSimulation) {
    foeRigidBodyPool componentPool = (foeRigidBodyPool)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
    if (componentPool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_BINARY_ERROR_RIGID_BODY_POOL_NOT_FOUND);

    foeRigidBody componentData;
    foeResultSet result =
        binary_read_foeRigidBody(pReadBuffer, pReadSize, groupTranslator, &componentData);
    if (result.value == FOE_SUCCESS)
        foeEcsComponentPoolInsert(componentPool, entity, &componentData);

    return result;
}