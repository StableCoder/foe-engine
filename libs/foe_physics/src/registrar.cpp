/*
    Copyright (C) 2021 George Cave.

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

#include <foe/physics/registrar.hpp>

#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/system.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/core.hpp>
#include <foe/simulation/state.hpp>

#include "log.hpp"

namespace {

void onCreate(foeSimulationState *pSimulationState) {
    auto *pCollisionShapeLoader = new foePhysCollisionShapeLoader;

    // Resources
    pSimulationState->resourcePools.emplace_back(
        new foePhysCollisionShapePool{pCollisionShapeLoader});

    // Loaders
    pSimulationState->resourceLoaders.emplace_back(pCollisionShapeLoader);

    // Components
    pSimulationState->componentPools.emplace_back(new foeRigidBodyPool);

    // Systems
    pSimulationState->systems.emplace_back(new foePhysicsSystem);
}

template <typename DestroyType, typename InType>
void searchAndDestroy(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr) {
        delete dynPtr;
        ptr = nullptr;
    }
}

void onDestroy(foeSimulationState *pSimulationState) {
    // Systems
    for (auto &pSystem : pSimulationState->systems) {
        searchAndDestroy<foePhysicsSystem>(pSystem);
    }

    // Components
    for (auto &pPool : pSimulationState->componentPools) {
        searchAndDestroy<foeRigidBodyPool>(pPool);
    }

    // Loaders
    for (auto &pLoader : pSimulationState->resourceLoaders) {
        searchAndDestroy<foePhysCollisionShapeLoader>(pLoader);
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy<foePhysCollisionShapePool>(pPool);
    }
}

template <typename SearchType, typename InputIt>
SearchType *search(InputIt start, InputIt end) noexcept {
    for (; start != end; ++start) {
        auto *dynPtr = dynamic_cast<SearchType *>(*start);
        if (dynPtr)
            return dynPtr;
    }

    return nullptr;
}

void onInitialization(foeSimulationInitInfo const *pInitInfo) {
    { // Loaders
        auto *pIt = pInitInfo->pResourceLoaders;
        auto const *pEndIt = pInitInfo->pResourceLoaders + pInitInfo->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pCollisionShapeLoader = dynamic_cast<foePhysCollisionShapeLoader *>(*pIt);
            if (pCollisionShapeLoader) {
                pCollisionShapeLoader->initialize(pInitInfo->resourceDefinitionImportFn,
                                                  pInitInfo->asyncJobFn);
            }
        }
    }

    { // Systems
        auto *pIt = pInitInfo->pSystems;
        auto const *pEndIt = pInitInfo->pSystems + pInitInfo->systemCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pPhysicsSystem = dynamic_cast<foePhysicsSystem *>(*pIt);
            if (pPhysicsSystem) {
                auto *pCollisionShapeLoader = search<foePhysCollisionShapeLoader>(
                    pInitInfo->pResourceLoaders,
                    pInitInfo->pResourceLoaders + pInitInfo->resourceLoaderCount);

                auto *pCollisionShapePool = search<foePhysCollisionShapePool>(
                    pInitInfo->pResourcePools,
                    pInitInfo->pResourcePools + pInitInfo->resourcePoolCount);

                auto *pRigidBodyPool = search<foeRigidBodyPool>(pInitInfo->pComponentPools,
                                                                pInitInfo->pComponentPools +
                                                                    pInitInfo->componentPoolCount);

                auto *pPosition3dPool = search<foePosition3dPool>(
                    pInitInfo->pComponentPools,
                    pInitInfo->pComponentPools + pInitInfo->componentPoolCount);

                pPhysicsSystem->initialize(pCollisionShapeLoader, pCollisionShapePool,
                                           pRigidBodyPool, pPosition3dPool);
            }
        }
    }
}

template <typename DestroyType, typename InType>
void searchAndDeinit(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr) {
        dynPtr->deinitialize();
    }
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Systems
    for (auto *pSystem : pSimulationState->systems) {
        searchAndDeinit<foePhysicsSystem>(pSystem);
    }

    // Loaders
    for (auto *pLoader : pSimulationState->resourceLoaders) {
        searchAndDeinit<foePhysCollisionShapeLoader>(pLoader);
    }
}

} // namespace

void foePhysicsRegisterFunctionality() {
    FOE_LOG(foePhysics, Verbose,
            "foePhysicsRegisterFunctionality - Starting to register functionality")

    foeRegisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    FOE_LOG(foePhysics, Verbose,
            "foePhysicsRegisterFunctionality - Completed registering functionality")
}

void foePhysicsDeregisterFunctionality() {
    FOE_LOG(foePhysics, Verbose,
            "foePhysicsDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    FOE_LOG(foePhysics, Verbose,
            "foePhysicsDeregisterFunctionality - Completed deregistering functionality")
}