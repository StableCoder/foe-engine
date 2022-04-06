/*
    Copyright (C) 2021-2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <foe/position/registration.h>

#include <foe/position/component/3d_pool.hpp>
#include <foe/position/type_defs.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "log.hpp"

namespace {

auto create(foeSimulation *pSimulation) -> std::error_code {
    std::error_code errC;

    // Components Pools
    if (foeSimulationIncrementRefCount(pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL,
            .pComponentPool = new foePosition3dPool,
            .pMaintenanceFn = [](void *pData) { ((foePosition3dPool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (errC) {
            delete (foePosition3dPool *)createInfo.pComponentPool;
            FOE_LOG(foePosition, Error,
                    "create - Failed to create foePosition3dPool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL,
                                       nullptr);
    }

CREATE_FAILED:

    return errC;
}

size_t destroy(foeSimulation *pSimulation) {
    size_t count;
    size_t errors = 0;

    // Component Pools
    std::error_code errC = foeSimulationDecrementRefCount(
        pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL, &count);
    if (errC) {
        // Trying to destroy something that doesn't exist? Not optimal
        FOE_LOG(foePosition, Warning,
                "Attempted to decrement/destroy foePosition3dPool that doesn't exist - {}",
                errC.message());
        ++errors;
    } else if (count == 0) {
        foePosition3dPool *pData;
        errC = foeSimulationReleaseComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL, (void **)&pData);
        if (errC) {
            FOE_LOG(foePosition, Warning, "Could not release foePosition3dPool to destroy - {}",
                    errC.message());
            ++errors;
        } else {
            delete pData;
        }
    }

    return errors;
}

} // namespace

int foePositionFunctionalityID() { return FOE_POSITION_FUNCTIONALITY_ID; }

extern "C" foeErrorCode foePositionRegisterFunctionality() {
    FOE_LOG(foePosition, Verbose,
            "foePositionRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foePositionFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
    });

    if (errC) {
        FOE_LOG(foePosition, Error,
                "foePositionRegisterFunctionality - Failed registering functionality: {} - {}",
                errC.value(), errC.message())
    } else {
        FOE_LOG(foePosition, Verbose,
                "foePositionRegisterFunctionality - Completed registering functionality")
    }

    return foeToErrorCode(errC);
}

extern "C" void foePositionDeregisterFunctionality() {
    FOE_LOG(foePosition, Verbose,
            "foePositionDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foePositionFunctionalityID());

    FOE_LOG(foePosition, Verbose,
            "foePositionDeregisterFunctionality - Completed deregistering functionality")
}