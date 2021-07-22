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
#include <foe/simulation/registration.hpp>
#include <foe/simulation/registration_fn_templates.hpp>
#include <foe/simulation/simulation.hpp>

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

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pCollisionShape, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_RESOURCE_ERROR_IMPORT_FAILED);
}

void onCreate(foeSimulationState *pSimulationState) {
    // Resources
    if (auto *pPool = search<foeCollisionShapePool>(pSimulationState->resourcePools.begin(),
                                                    pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeCollisionShapePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = collisionShapeLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    // Loaders
    if (auto *pLoader = searchLoaders<foeCollisionShapeLoader>(
            pSimulationState->resourceLoaders.begin(), pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeCollisionShapeLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pCollisionShapeLoader =
                        reinterpret_cast<foeCollisionShapeLoader *>(pLoader);
                    pCollisionShapeLoader->maintenance();
                },
        });
    }

    // Components
    if (auto *pPool = search<foeRigidBodyPool>(pSimulationState->componentPools.begin(),
                                               pSimulationState->componentPools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeRigidBodyPool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    // Systems
    pSimulationState->systems.emplace_back(new foePhysicsSystem);
}

void onDestroy(foeSimulationState *pSimulationState) {
    // Systems
    for (auto &pSystem : pSimulationState->systems) {
        searchAndDestroy<foePhysicsSystem>(pSystem);
    }

    // Components
    for (auto &pPool : pSimulationState->componentPools) {
        searchAndDestroy2<foeRigidBodyPool>(pPool);
    }

    // Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy2<foeCollisionShapeLoader>(it.pLoader);
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy2<foeCollisionShapePool>(pPool);
    }
}

struct Initialized {
    bool collisionShape;
};

void deinitialize(Initialized const &initialized, foeSimulationStateLists const *pSimStateData) {
    // Loaders
    auto *pIt = pSimStateData->pResourceLoaders;
    auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

    for (; pIt != pEndIt; ++pIt) {
        if (initialized.collisionShape)
            searchAndDeinit<foeCollisionShapeLoader>(pIt->pLoader);
    }
}

std::error_code onInitialization(foeSimulationInitInfo const *pInitInfo,
                                 foeSimulationStateLists const *pSimStateData) {
    std::error_code errC;
    Initialized initialized{};

    { // Loaders
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pCollisionShapeLoader = dynamic_cast<foeCollisionShapeLoader *>(pIt->pLoader);
            if (pCollisionShapeLoader) {
                ++pCollisionShapeLoader->initCount;
                if (pCollisionShapeLoader->initialized())
                    continue;

                errC = pCollisionShapeLoader->initialize();
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.collisionShape = true;
            }
        }
    }

    { // Systems
        auto *pIt = pSimStateData->pSystems;
        auto const *pEndIt = pSimStateData->pSystems + pSimStateData->systemCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pPhysicsSystem = dynamic_cast<foePhysicsSystem *>(*pIt);
            if (pPhysicsSystem) {
                auto *pCollisionShapeLoader = searchLoaders<foeCollisionShapeLoader>(
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

INITIALIZATION_FAILED:
    if (errC)
        deinitialize(initialized, pSimStateData);

    return errC;
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Systems
    for (auto *pSystem : pSimulationState->systems) {
        searchAndDeinit2<foePhysicsSystem>(pSystem);
    }

    // Loaders
    for (auto const &it : pSimulationState->resourceLoaders) {
        searchAndDeinit<foeCollisionShapeLoader>(it.pLoader);
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