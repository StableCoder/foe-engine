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
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/system.hpp>
#include <foe/position/component/3d_pool.hpp>
#include <foe/resource/error_code.hpp>
#include <foe/simulation/core.hpp>
#include <foe/simulation/state.hpp>

#include "log.hpp"

namespace {

foeResourceCreateInfoBase *importFn(void *pContext, foeResourceID resource) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);
    return pGroupData->getResourceDefinition(resource);
}

void collisionShapeLoadFn(void *pContext,
                          void *pResource,
                          void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pCollisionShape = reinterpret_cast<foeCollisionShape *>(pResource);

    auto pLocalCreateInfo = pCollisionShape->pCreateInfo;

    for (auto *it : pSimulationState->resourceLoaders) {
        if (it->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it->load(pCollisionShape, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_RESOURCE_ERROR_IMPORT_FAILED);
}

void onCreate(foeSimulationState *pSimulationState) {
    // Resources
    pSimulationState->resourcePools.emplace_back(new foeCollisionShapePool{foeResourceFns{
        .pImportContext = &pSimulationState->groupData,
        .pImportFn = importFn,
        .pLoadContext = pSimulationState,
        .pLoadFn = collisionShapeLoadFn,
        .asyncTaskFn = pSimulationState->createInfo.asyncJobFn,
    }});

    // Loaders
    pSimulationState->resourceLoaders.emplace_back(new foeCollisionShapeLoader);

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
        searchAndDestroy<foeCollisionShapeLoader>(pLoader);
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy<foeCollisionShapePool>(pPool);
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

void onInitialization(foeSimulationInitInfo const *pInitInfo,
                      foeSimulationStateLists const *pSimStateData) {
    { // Loaders
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pCollisionShapeLoader = dynamic_cast<foeCollisionShapeLoader *>(*pIt);
            if (pCollisionShapeLoader) {
                pCollisionShapeLoader->initialize();
            }
        }
    }

    { // Systems
        auto *pIt = pSimStateData->pSystems;
        auto const *pEndIt = pSimStateData->pSystems + pSimStateData->systemCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pPhysicsSystem = dynamic_cast<foePhysicsSystem *>(*pIt);
            if (pPhysicsSystem) {
                auto *pCollisionShapeLoader = search<foeCollisionShapeLoader>(
                    pSimStateData->pResourceLoaders,
                    pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount);

                auto *pCollisionShapePool = search<foeCollisionShapePool>(
                    pSimStateData->pResourcePools,
                    pSimStateData->pResourcePools + pSimStateData->resourcePoolCount);

                auto *pRigidBodyPool = search<foeRigidBodyPool>(
                    pSimStateData->pComponentPools,
                    pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

                auto *pPosition3dPool = search<foePosition3dPool>(
                    pSimStateData->pComponentPools,
                    pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

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
        searchAndDeinit<foeCollisionShapeLoader>(pLoader);
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