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

struct TypeSelection {
    // Resources
    bool collisionShapeResource;
    // Loaders
    bool collisionShapeLoader;
    // Components
    bool rigidBodyComponent;
    // Systems
    bool physicsSystem;
};

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

bool destroySelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    bool issues = false;

    // Systems
    for (auto &pSystem : pSimulationState->systems) {
        if (pSystem == nullptr)
            continue;

        if ((pSelection == nullptr || pSelection->physicsSystem) &&
            pSystem->sType == FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM &&
            --pSystem->refCount == 0) {
            delete (foePhysicsSystem *)pSystem;
            pSystem = nullptr;
        }
    }

    // Components
    if (pSelection == nullptr || pSelection->rigidBodyComponent) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL, &count);
        if (errC) {
            FOE_LOG(foePhysics, Warning,
                    "Attempted to decrement/destroy foeRigidBodyPool that doesn't exist - {}",
                    errC.message());
            issues = true;
        } else if (count == 0) {
            foeRigidBodyPool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foePhysics, Warning, "Could not release foeRigidBodyPool to destroy - {}",
                        errC.message());
                issues = true;
            } else {
                delete pData;
            }
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->collisionShapeLoader) {
        errC = foeSimulationDecrementRefCount(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
        if (errC) {
            // Trying to destroy something that doesn't exist? Not optimal
            FOE_LOG(
                foePhysics, Warning,
                "Attempted to decrement/destroy foeCollisionShapeLoader that doesn't exist - {}",
                errC.message());
            issues = true;
        } else if (count == 0) {
            foeCollisionShapeLoader *pLoader;
            errC = foeSimulationReleaseResourceLoader(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER,
                (void **)&pLoader);
            if (errC) {
                FOE_LOG(foePhysics, Warning,
                        "Could not release foeCollisionShapeLoader to destroy - {}",
                        errC.message());
                issues = true;
            } else {
                delete pLoader;
            }
        }
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if ((pSelection == nullptr || pSelection->collisionShapeResource) &&
            pPool->sType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL) {
            auto *pTemp = (foeCollisionShapePool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        }
    }

    return !issues;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool->sType == FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL) {
            ++pPool->refCount;
            selection.collisionShapeResource = true;
        }
    }

    if (!selection.collisionShapeResource) {
        auto *pPool = new foeCollisionShapePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = collisionShapeLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
        selection.collisionShapeResource = true;
    }

    // Loaders
    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER,
            .pLoader = new foeCollisionShapeLoader,
            .pCanProcessCreateInfoFn = foeCollisionShapeLoader::canProcessCreateInfo,
            .pLoadFn = foeCollisionShapeLoader::load,
            .pMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeCollisionShapeLoader *>(pLoader)->maintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeCollisionShapeLoader *)loaderCI.pLoader;
            FOE_LOG(
                foePhysics, Error,
                "onCreate - Failed to create foeCollisionShapeLoader on Simulation {} due to {}",
                (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        if (foeSimulationIncrementRefCount(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, nullptr)) {
            FOE_LOG(foePhysics, Error,
                    "onCreate - Failed to increment newly created foeCollisionShapeLoader on "
                    "Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
        }
    }
    selection.collisionShapeLoader = true;

    // Components
    if (foeSimulationIncrementRefCount(pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
            .pComponentPool = new foeRigidBodyPool,
            .pMaintenanceFn = [](void *pData) { ((foeRigidBodyPool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeRigidBodyPool *)createInfo.pComponentPool;
            FOE_LOG(foePhysics, Error,
                    "onCreate - Failed to create foeRigidBodyPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
                                       nullptr);
    }
    selection.rigidBodyComponent = true;

    // Systems
    if (auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        pSystem) {
        ++pSystem->refCount;
        selection.physicsSystem = true;
    } else {
        pSystem = new foePhysicsSystem;
        ++pSystem->refCount;
        pSimulationState->systems.emplace_back(pSystem);
        selection.physicsSystem = true;
    }

CREATE_FAILED:
    if (errC)
        destroySelection(pSimulationState, &selection);

    return errC;
}

bool destroy(foeSimulationState *pSimulationState) {
    return destroySelection(pSimulationState, nullptr);
}

bool deinitializeSelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;

    // Systems
    if (pSelection == nullptr || pSelection->physicsSystem) {
        auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        if (pSystem != nullptr && --pSystem->initCount == 0) {
            pSystem->deinitialize();
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->collisionShapeLoader) {
        errC = foeSimulationDecrementInitCount(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
        if (errC) {
            FOE_LOG(foePhysics, Warning,
                    "Failed to decrement foeArmatureLoader initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulationState, errC.message());
        } else if (count == 0) {
            auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
            pLoader->deinitialize();
        }
    }

    return true;
}

auto initialize(foeSimulationState *pSimulationState, foeSimulationInitInfo const *pInitInfo)
    -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection{};

    // Loaders
    errC =
        foeSimulationIncrementInitCount((foeSimulationState *)pSimulationState,
                                        FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
    if (errC) {
        FOE_LOG(foePhysics, Error,
                "Failed to increment foeArmatureLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.collisionShapeLoader = true;
        auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
        errC = pLoader->initialize();
        if (errC) {
            FOE_LOG(foePhysics, Error,
                    "Failed to initialize foeArmatureLoader on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    if (auto *pPhysicsSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        pPhysicsSystem) {
        if (!pPhysicsSystem->initialized()) {
            auto *pCollisionShapeLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);

            auto *pCollisionShapePool = (foeCollisionShapePool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL);

            auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);

            auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

            errC = pPhysicsSystem->initialize(pCollisionShapeLoader, pCollisionShapePool,
                                              pRigidBodyPool, pPosition3dPool);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pPhysicsSystem->initCount;
        selection.physicsSystem = true;
    }

INITIALIZATION_FAILED:
    if (errC) {
        deinitializeSelection(pSimulationState, &selection);
    }

    return errC;
}

bool deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelection(pSimulationState, nullptr);
}

} // namespace

int foePhysicsFunctionalityID() { return FOE_PHYSICS_FUNCTIONALITY_ID; }

auto foePhysicsRegisterFunctionality() -> std::error_code {
    FOE_LOG(foePhysics, Verbose,
            "foePhysicsRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foePhysicsFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
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

    foeDeregisterFunctionality(foePhysicsFunctionalityID());

    FOE_LOG(foePhysics, Verbose,
            "foePhysicsDeregisterFunctionality - Completed deregistering functionality")
}