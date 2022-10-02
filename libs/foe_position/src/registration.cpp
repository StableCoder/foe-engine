// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/position/registration.h>

#include <foe/position/component/3d_pool.hpp>
#include <foe/position/type_defs.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "log.hpp"
#include "result.h"

namespace {

foeResultSet create(foeSimulation *pSimulation) {
    foeResultSet result;

    // Components Pools
    result = foeSimulationIncrementRefCount(pSimulation,
                                            FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL,
            .pComponentPool = new (std::nothrow) foePosition3dPool,
            .pMaintenanceFn = [](void *pData) { ((foePosition3dPool *)pData)->maintenance(); },
        };
        if (createInfo.pComponentPool == nullptr) {
            result = to_foeResult(FOE_POSITION_ERROR_OUT_OF_MEMORY);
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foePosition3dPool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePosition, Error,
                    "create - Failed to create foePosition3dPool on Simulation {} due to {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL,
                                       nullptr);
    }

CREATE_FAILED:
    return result;
}

size_t destroy(foeSimulation *pSimulation) {
    size_t count;
    size_t errors = 0;
    foeResultSet result;

    // Component Pools
    result = foeSimulationDecrementRefCount(pSimulation,
                                            FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL, &count);
    if (result.value != FOE_SUCCESS) {
        // Trying to destroy something that doesn't exist? Not optimal
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foePosition, Warning,
                "Attempted to decrement/destroy foePosition3dPool that doesn't exist - {}", buffer);

        ++errors;
    } else if (count == 0) {
        foePosition3dPool *pData;
        result = foeSimulationReleaseComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL, (void **)&pData);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePosition, Warning, "Could not release foePosition3dPool to destroy - {}",
                    buffer);

            ++errors;
        } else {
            delete pData;
        }
    }

    return errors;
}

} // namespace

int foePositionFunctionalityID() { return FOE_POSITION_LIBRARY_ID; }

extern "C" foeResultSet foePositionRegisterFunctionality() {
    FOE_LOG(foePosition, Verbose,
            "foePositionRegisterFunctionality - Starting to register functionality")

    foeResultSet result = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foePositionFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
    });

    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foePosition, Error,
                "foePositionRegisterFunctionality - Failed registering functionality: {}", buffer)
    } else {
        FOE_LOG(foePosition, Verbose,
                "foePositionRegisterFunctionality - Completed registering functionality")
    }

    return result;
}

extern "C" void foePositionDeregisterFunctionality() {
    FOE_LOG(foePosition, Verbose,
            "foePositionDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foePositionFunctionalityID());

    FOE_LOG(foePosition, Verbose,
            "foePositionDeregisterFunctionality - Completed deregistering functionality")
}