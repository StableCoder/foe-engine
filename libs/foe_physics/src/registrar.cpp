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
                          foeResource resource,
                          PFN_foeResourcePostLoad *pPostLoadFn) {
    auto *pSimulation = reinterpret_cast<foeSimulation *>(pContext);

    auto pLocalCreateInfo = foeResourceGetCreateInfo(resource);

    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            it.pLoadFn(it.pLoader, resource, pLocalCreateInfo, pPostLoadFn);
            return;
        }
    }

    pPostLoadFn(resource, foeToErrorCode(FOE_PHYSICS_ERROR_IMPORT_FAILED), nullptr, nullptr,
                nullptr, nullptr, nullptr);
}

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->physicsSystem) {
        errC = foeSimulationDecrementRefCount(pSimulation,
                                              FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foePhysics, Warning,
                    "Attempted to decrement/destroy foePhysicsSystem that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foePhysicsSystem *pData;
            errC = foeSimulationReleaseSystem(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM, (void **)&pData);
            if (errC) {
                FOE_LOG(foePhysics, Warning, "Could not release foePhysicsSystem to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Components
    if (pSelection == nullptr || pSelection->rigidBodyComponent) {
        errC = foeSimulationDecrementRefCount(pSimulation,
                                              FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL, &count);
        if (errC) {
            FOE_LOG(foePhysics, Warning,
                    "Attempted to decrement/destroy foeRigidBodyPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeRigidBodyPool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foePhysics, Warning, "Could not release foeRigidBodyPool to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->collisionShapeLoader) {
        errC = foeSimulationDecrementRefCount(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
        if (errC) {
            // Trying to destroy something that doesn't exist? Not optimal
            FOE_LOG(
                foePhysics, Warning,
                "Attempted to decrement/destroy foeCollisionShapeLoader that doesn't exist - {}",
                errC.message());
            ++errors;
        } else if (count == 0) {
            foeCollisionShapeLoader *pLoader;
            errC = foeSimulationReleaseResourceLoader(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, (void **)&pLoader);
            if (errC) {
                FOE_LOG(foePhysics, Warning,
                        "Could not release foeCollisionShapeLoader to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pLoader;
            }
        }
    }

    // Resources
    if (pSelection == nullptr || pSelection->collisionShapeResource) {
        errC = foeSimulationDecrementRefCount(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL, &count);
        if (errC) {
            // Trying to destroy something that doesn't exist? Not optimal
            FOE_LOG(foePhysics, Warning,
                    "Attempted to decrement/destroy foeCollisionShapePool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeCollisionShapePool *pItem;
            errC = foeSimulationReleaseResourcePool(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL, (void **)&pItem);
            if (errC) {
                FOE_LOG(foePhysics, Warning,
                        "Could not release foeCollisionShapePool to destroy - {}", errC.message());
                ++errors;
            } else {
                delete pItem;
            }
        }
    }

    return errors;
}

auto create(foeSimulation *pSimulation) -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Resources
    if (foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL,
                                       nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationResourcePoolData createInfo{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL,
            .pResourcePool = new foeCollisionShapePool{foeResourceFns{
                .pImportContext = &pSimulation->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulation,
                .pLoadFn = collisionShapeLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulation, &createInfo);
        if (errC) {
            delete (foeCollisionShapePool *)createInfo.pResourcePool;
            FOE_LOG(foePhysics, Error,
                    "onCreate - Failed to create foeCollisionShapePool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL,
                                       nullptr);
    }
    selection.collisionShapeResource = true;

    // Loaders
    if (foeSimulationIncrementRefCount(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, nullptr)) {
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeCollisionShapeLoader *)loaderCI.pLoader;
            FOE_LOG(
                foePhysics, Error,
                "onCreate - Failed to create foeCollisionShapeLoader on Simulation {} due to {}",
                (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        if (foeSimulationIncrementRefCount(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, nullptr)) {
            FOE_LOG(foePhysics, Error,
                    "onCreate - Failed to increment newly created foeCollisionShapeLoader on "
                    "Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
        }
    }
    selection.collisionShapeLoader = true;

    // Components
    if (foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
            .pComponentPool = new foeRigidBodyPool,
            .pMaintenanceFn = [](void *pData) { ((foeRigidBodyPool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (errC) {
            delete (foeRigidBodyPool *)createInfo.pComponentPool;
            FOE_LOG(foePhysics, Error,
                    "onCreate - Failed to create foeRigidBodyPool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
                                       nullptr);
    }
    selection.rigidBodyComponent = true;

    // Systems
    if (foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
                                       nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
            .pSystem = new foePhysicsSystem,
        };
        errC = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (errC) {
            delete (foePhysicsSystem *)createInfo.pSystem;
            FOE_LOG(foePhysics, Error,
                    "onCreate - Failed to create foePhysicsSystem on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
                                       nullptr);
    }
    selection.physicsSystem = true;

CREATE_FAILED:
    if (errC) {
        size_t errors = destroySelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foePhysics, Warning, "Encountered {} issues destroying after failed creation.",
                    errors);
    }

    return errC;
}

size_t destroy(foeSimulation *pSimulation) { return destroySelection(pSimulation, nullptr); }

size_t deinitializeSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->physicsSystem) {
        errC = foeSimulationDecrementInitCount(pSimulation,
                                               FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foePhysics, Warning,
                    "Failed to decrement foePhysicsSystem initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foePhysicsSystem *)foeSimulationGetSystem(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
            pLoader->deinitialize();
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->collisionShapeLoader) {
        errC = foeSimulationDecrementInitCount(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
        if (errC) {
            FOE_LOG(
                foePhysics, Warning,
                "Failed to decrement foeCollisionShapeLoader initialization count on Simulation {} "
                "with error {}",
                (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
            pLoader->deinitialize();
        }
    }

    return errors;
}

auto initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo)
    -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection{};

    // Loaders
    errC = foeSimulationIncrementInitCount(
        (foeSimulation *)pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
    if (errC) {
        FOE_LOG(foePhysics, Error,
                "Failed to increment foeArmatureLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.collisionShapeLoader = true;
    if (count == 1) {
        auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
        errC = pLoader->initialize();
        if (errC) {
            FOE_LOG(foePhysics, Error,
                    "Failed to initialize foeArmatureLoader on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    errC = foeSimulationIncrementInitCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
                                           &count);
    if (errC) {
        FOE_LOG(foePhysics, Error,
                "Failed to increment foePhysicsSystem initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.physicsSystem = true;
    if (count == 1) {
        auto *pCollisionShapeLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
        auto *pCollisionShapePool = (foeCollisionShapePool *)foeSimulationGetResourcePool(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL);
        auto *pRigidBodyPool = (foeRigidBodyPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
        errC = pSystem->initialize(pCollisionShapeLoader, pCollisionShapePool, pRigidBodyPool,
                                   pPosition3dPool);
        if (errC) {
            FOE_LOG(foePhysics, Error,
                    "Failed to initialize foePhysicsSystem on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foePhysics, Warning,
                    "Encountered {} issues deinitializing after failed initialization", errors);
    }

    return errC;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
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