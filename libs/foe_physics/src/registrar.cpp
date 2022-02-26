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

#include <foe/physics/registrar.hpp>

#include <foe/physics/component/rigid_body_pool.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/resource/collision_shape_pool.hpp>
#include <foe/physics/system.hpp>
#include <foe/physics/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/registration_fn_templates.hpp>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
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

    pPostLoadFn(pResource, FOE_PHYSICS_ERROR_IMPORT_FAILED);
}

void onCreate(foeSimulationState *pSimulationState) {
    // Resources
    bool poolFound = false;

    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool->sType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL) {
            ++pPool->refCount;
            poolFound = true;
        }
    }

    if (!poolFound) {
        auto *pPool = new foeCollisionShapePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = collisionShapeLoadFn,
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
    if (auto *pSystem = search<foePhysicsSystem>(pSimulationState->systems.begin(),
                                                 pSimulationState->systems.end());
        pSystem) {
        ++pSystem->refCount;
    } else {
        pSystem = new foePhysicsSystem;
        ++pSystem->refCount;
        pSimulationState->systems.emplace_back(pSystem);
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
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy<foeCollisionShapeLoader>(it.pLoader);
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL) {
            auto *pTemp = (foeCollisionShapePool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        }
    }
}

struct Initialized {
    // Loaders
    bool collisionShape;
    // Systems
    bool physics;
};

void deinitialize(foeSimulationState const *pSimulationState, Initialized const &initialized) {
    { // Systems
        auto *pIt = pSimulationState->systems.data();
        auto const *pEndIt = pIt + pSimulationState->systems.size();

        for (; pIt != pEndIt; ++pIt) {
            if (initialized.physics)
                searchAndDeinit<foePhysicsSystem>(*pIt);
        }
    }

    { // Loaders
        auto *pIt = pSimulationState->resourceLoaders.data();
        auto const *pEndIt = pIt + pSimulationState->resourceLoaders.size();

        for (; pIt != pEndIt; ++pIt) {
            if (initialized.collisionShape)
                searchAndDeinit<foeCollisionShapeLoader>(pIt->pLoader);
        }
    }
}

std::error_code onInitialization(foeSimulationState const *pSimulationState,
                                 foeSimulationInitInfo const *pInitInfo) {
    std::error_code errC;
    Initialized initialized{};

    { // Loaders
        auto *pIt = pSimulationState->resourceLoaders.data();
        auto const *pEndIt = pIt + pSimulationState->resourceLoaders.size();

        for (; pIt != pEndIt; ++pIt) {
            if (auto *pCollisionShapeLoader = dynamic_cast<foeCollisionShapeLoader *>(pIt->pLoader);
                pCollisionShapeLoader) {
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
        auto *pIt = pSimulationState->systems.data();
        auto const *pEndIt = pIt + pSimulationState->systems.size();

        for (; pIt != pEndIt; ++pIt) {
            if (auto *pPhysicsSystem = dynamic_cast<foePhysicsSystem *>(*pIt); pPhysicsSystem) {
                ++pPhysicsSystem->initCount;
                if (pPhysicsSystem->initialized())
                    continue;

                auto *pCollisionShapeLoader = searchLoaders<foeCollisionShapeLoader>(
                    pSimulationState->resourceLoaders.data(),
                    pSimulationState->resourceLoaders.data() +
                        pSimulationState->resourceLoaders.size());

                foeCollisionShapePool *pCollisionShapePool{nullptr};

                auto *it = pSimulationState->resourcePools.data();
                auto *endIt = it + pSimulationState->resourcePools.size();
                for (; it != endIt; ++it) {
                    if (*it == nullptr)
                        continue;

                    if ((*it)->sType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL)
                        pCollisionShapePool = (foeCollisionShapePool *)(*it);
                }

                auto *pRigidBodyPool =
                    search<foeRigidBodyPool>(pSimulationState->componentPools.data(),
                                             pSimulationState->componentPools.data() +
                                                 pSimulationState->componentPools.size());

                auto *pPosition3dPool =
                    search<foePosition3dPool>(pSimulationState->componentPools.data(),
                                              pSimulationState->componentPools.data() +
                                                  pSimulationState->componentPools.size());

                pPhysicsSystem->initialize(pCollisionShapeLoader, pCollisionShapePool,
                                           pRigidBodyPool, pPosition3dPool);
                initialized.physics = true;
            }
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitialize(pSimulationState, initialized);

    return errC;
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Systems
    for (auto *pSystem : pSimulationState->systems) {
        searchAndDeinit<foePhysicsSystem>(pSystem);
    }

    // Loaders
    for (auto const &it : pSimulationState->resourceLoaders) {
        searchAndDeinit<foeCollisionShapeLoader>(it.pLoader);
    }
}

} // namespace

int foePhysicsFunctionalityID() { return FOE_PHYSICS_FUNCTIONALITY_ID; }

auto foePhysicsRegisterFunctionality() -> std::error_code {
    FOE_LOG(foePhysics, Verbose,
            "foePhysicsRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foePhysicsFunctionalityID(),
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    if (errC) {
        FOE_LOG(foePhysics, Error,
                "foePhysicsRegisterFunctionality - Failed registering functionality: {} - {}",
                errC.value(), errC.message())
    } else {
        FOE_LOG(foePhysics, Verbose,
                "foePhysicsRegisterFunctionality - Completed registering functionality")
    }

    return errC;
}

void foePhysicsDeregisterFunctionality() {
    FOE_LOG(foePhysics, Verbose,
            "foePhysicsDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .id = foePhysicsFunctionalityID(),
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    FOE_LOG(foePhysics, Verbose,
            "foePhysicsDeregisterFunctionality - Completed deregistering functionality")
}