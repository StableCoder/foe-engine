// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "rigid_body.h"

#include <foe/physics/binary.h>
#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/simulation/simulation.hpp>

#include "result.h"

extern "C" foeResultSet export_foeRigidBody(foeEntityID entity,
                                            foeSimulation const *pSimulation,
                                            foeImexBinarySet *pBinarySets) {
    foeResultSet result = to_foeResult(FOE_PHYSICS_BINARY_DATA_NOT_EXPORTED);
    foeImexBinarySet set = {};

    auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
    if (pRigidBodyPool) {
        if (auto offset = pRigidBodyPool->find(entity); offset != pRigidBodyPool->size()) {
            auto *pRigidBodyData = pRigidBodyPool->begin<1>() + offset;

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
    auto *pComponentPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
    if (pComponentPool == nullptr)
        return to_foeResult(FOE_PHYSICS_BINARY_ERROR_RIGID_BODY_POOL_NOT_FOUND);

    foeRigidBody componentData;
    foeResultSet result =
        binary_read_foeRigidBody(pReadBuffer, pReadSize, groupTranslator, &componentData);
    if (result.value == FOE_SUCCESS)
        pComponentPool->insert(entity, std::move(componentData));

    return result;
}