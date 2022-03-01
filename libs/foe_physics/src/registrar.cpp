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
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pCollisionShape, pLocalCreateInfo, pPostLoadFn);
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
    if (auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeCollisionShapeLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pCanProcessCreateInfoFn = foeCollisionShapeLoader::canProcessCreateInfo,
            .pLoadFn = foeCollisionShapeLoader::load,
            .pMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pCollisionShapeLoader =
                        reinterpret_cast<foeCollisionShapeLoader *>(pLoader);
                    pCollisionShapeLoader->maintenance();
                },
        });
    }

    // Components
    if (auto *pPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeRigidBodyPool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    // Systems
    if (auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
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
        if (pSystem == nullptr)
            continue;

        if (pSystem->sType == FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM &&
            --pSystem->refCount == 0) {
            delete (foePhysicsSystem *)pSystem;
            pSystem = nullptr;
        }
    }

    // Components
    for (auto &pPool : pSimulationState->componentPools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL && --pPool->refCount == 0) {
            delete (foeRigidBodyPool *)pPool;
            pPool = nullptr;
        }
    }

    // Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        if (it.pLoader == nullptr)
            continue;

        if (it.pLoader->sType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER &&
            --it.pLoader->refCount == 0) {
            delete (foeCollisionShapeLoader *)it.pLoader;
            it.pLoader = nullptr;
        }
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
    // Systems
    if (initialized.physics) {
        auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        if (pSystem != nullptr && --pSystem->initCount == 0) {
            pSystem->deinitialize();
        }
    }

    // Loaders
    if (initialized.collisionShape) {
        if (auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
            pLoader != nullptr && --pLoader->initCount == 0) {
            pLoader->deinitialize();
        }
    }
}

std::error_code onInitialization(foeSimulationState const *pSimulationState,
                                 foeSimulationInitInfo const *pInitInfo) {
    std::error_code errC;
    Initialized initialized{};

    // Loaders
    if (auto *pCollisionShapeLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
        pCollisionShapeLoader) {
        ++pCollisionShapeLoader->initCount;
        initialized.collisionShape = true;
        if (!pCollisionShapeLoader->initialized()) {
            errC = pCollisionShapeLoader->initialize();
            if (errC)
                goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    if (auto *pPhysicsSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        pPhysicsSystem) {
        ++pPhysicsSystem->initCount;
        initialized.physics = true;
        if (!pPhysicsSystem->initialized()) {
            auto *pCollisionShapeLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);

            auto *pCollisionShapePool = (foeCollisionShapePool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL);

            auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

            auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

            pPhysicsSystem->initialize(pCollisionShapeLoader, pCollisionShapePool, pRigidBodyPool,
                                       pPosition3dPool);
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitialize(pSimulationState, initialized);

    return errC;
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Systems
    if (auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        pSystem != nullptr && --pSystem->initCount == 0) {
        pSystem->deinitialize();
    }

    // Loaders
    if (auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
        pLoader != nullptr && --pLoader->initCount == 0) {
        pLoader->deinitialize();
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