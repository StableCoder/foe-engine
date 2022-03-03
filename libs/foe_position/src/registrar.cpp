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

#include <foe/position/registrar.hpp>

#include <foe/position/component/3d_pool.hpp>
#include <foe/position/type_defs.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "log.hpp"

namespace {

auto onCreate(foeSimulationState *pSimulationState) -> std::error_code {
    // Components Pools
    if (auto *pPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
        pPool != nullptr) {
        ++pPool->refCount;
    } else {
        pPool = new foePosition3dPool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    return {};
}

auto onDestroy(foeSimulationState *pSimulationState) -> std::error_code {
    // Component Pools
    for (auto &pPool : pSimulationState->componentPools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL) {
            delete pPool;
            pPool = nullptr;
        }
    }

    return {};
}

} // namespace

int foePositionFunctionalityID() { return FOE_POSITION_FUNCTIONALITY_ID; }

auto foePositionRegisterFunctionality() -> std::error_code {
    FOE_LOG(foePosition, Verbose,
            "foePositionRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foePositionFunctionalityID(),
        .onCreate = onCreate,
        .onDestroy = onDestroy,
    });

    if (errC) {
        FOE_LOG(foePosition, Error,
                "foePositionRegisterFunctionality - Failed registering functionality: {} - {}",
                errC.value(), errC.message())
    } else {
        FOE_LOG(foePosition, Verbose,
                "foePositionRegisterFunctionality - Completed registering functionality")
    }

    return errC;
}

void foePositionDeregisterFunctionality() {
    FOE_LOG(foePosition, Verbose,
            "foePositionDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .id = foePositionFunctionalityID(),
        .onCreate = onCreate,
        .onDestroy = onDestroy,
    });

    FOE_LOG(foePosition, Verbose,
            "foePositionDeregisterFunctionality - Completed deregistering functionality")
}