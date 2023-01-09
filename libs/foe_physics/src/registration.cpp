// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/physics/registration.h>

#include <foe/physics/component/rigid_body_pool.h>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/system.hpp>
#include <foe/physics/type_defs.h>
#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource_fns.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "collision_shape_loader.hpp"

#include "log.hpp"
#include "result.h"

namespace {

struct TypeSelection {
    // Loaders
    bool collisionShapeLoader;
    // Components
    bool rigidBodyComponent;
    // Systems
    bool physicsSystem;
};

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    foeResultSet result;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->physicsSystem) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foePhysicsSystem that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foePhysicsSystem *pData;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                        "Could not release foePhysicsSystem to destroy: {}", buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Components
    if (pSelection == nullptr || pSelection->rigidBodyComponent) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeRigidBodyPool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeRigidBodyPool pool;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL, (void **)&pool);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeRigidBodyPool to destroy: {}", buffer);

                ++errors;
            } else {
                foeEcsDestroyComponentPool(pool);
            }
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->collisionShapeLoader) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
        if (result.value != FOE_SUCCESS) {
            // Trying to destroy something that doesn't exist? Not optimal
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeCollisionShapeLoader that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeCollisionShapeLoader *pLoader;
            result = foeSimulationReleaseResourceLoader(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, (void **)&pLoader);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeCollisionShapeLoader to destroy: {}", buffer);

                ++errors;
            } else {
                delete pLoader;
            }
        }
    }

    return errors;
}

foeResultSet create(foeSimulation *pSimulation) {
    foeResultSet result;
    TypeSelection selection = {};

    // Loaders
    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER,
            .pLoader = new (std::nothrow) foeCollisionShapeLoader,
            .pCanProcessCreateInfoFn = foeCollisionShapeLoader::canProcessCreateInfo,
            .pLoadFn = foeCollisionShapeLoader::load,
            .pMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeCollisionShapeLoader *>(pLoader)->maintenance();
                },
        };
        if (loaderCI.pLoader == nullptr)
            return to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);

        result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (result.value != FOE_SUCCESS) {
            delete (foeCollisionShapeLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeCollisionShapeLoader on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, nullptr);
    }
    selection.collisionShapeLoader = true;

    // Components
    result = foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
            .pMaintenanceFn =
                [](void *pData) { foeEcsComponentPoolMaintenance((foeEcsComponentPool)pData); },
        };

        result = foeEcsCreateComponentPool(0, 16, sizeof(foeRigidBody), NULL,
                                           (foeEcsComponentPool *)&createInfo.pComponentPool);
        if (result.value != FOE_SUCCESS) {
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeRigidBodyPool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeRigidBodyPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL,
                                       nullptr);
    }
    selection.rigidBodyComponent = true;

    // Systems
    result = foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
            .pSystem = new (std::nothrow) foePhysicsSystem,
        };
        if (createInfo.pSystem == nullptr) {
            result = to_foeResult(FOE_PHYSICS_ERROR_OUT_OF_MEMORY);
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foePhysicsSystem *)createInfo.pSystem;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foePhysicsSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
                                       nullptr);
    }
    selection.physicsSystem = true;

CREATE_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = destroySelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues destroying after failed creation.", errors);
    }

    return result;
}

size_t destroy(foeSimulation *pSimulation) { return destroySelection(pSimulation, nullptr); }

size_t deinitializeSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    foeResultSet result;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->physicsSystem) {
        result = foeSimulationDecrementInitCount(pSimulation,
                                                 FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(
                foePhysics, FOE_LOG_LEVEL_WARNING,
                "Failed to decrement foePhysicsSystem initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foePhysicsSystem *)foeSimulationGetSystem(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);
            pLoader->deinitialize();
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->collisionShapeLoader) {
        result = foeSimulationDecrementInitCount(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                    "Failed to decrement foeCollisionShapeLoader initialization count on "
                    "Simulation {}: {}",
                    (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
                pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);
            pLoader->deinitialize();
        }
    }

    return errors;
}

foeResultSet initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo) {
    foeResultSet result;
    size_t count;
    TypeSelection selection{};

    // Loaders
    result = foeSimulationIncrementInitCount(
        (foeSimulation *)pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeArmatureLoader initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);
        goto INITIALIZATION_FAILED;
    }
    selection.collisionShapeLoader = true;
    if (count == 1) {
        auto *pLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeArmatureLoader on Simulation {}: {}",
                    (void *)pSimulation, buffer);
            goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    result = foeSimulationIncrementInitCount(pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM,
                                             &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foePhysicsSystem initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.physicsSystem = true;
    if (count == 1) {
        auto *pCollisionShapeLoader = (foeCollisionShapeLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER);

        foeRigidBodyPool rigidBodyPool = (foeRigidBodyPool)foeSimulationGetComponentPool(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_RIGID_BODY_POOL);
        foePosition3dPool position3dPool = (foePosition3dPool)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto *pSystem = (foePhysicsSystem *)foeSimulationGetSystem(
            pSimulation, FOE_PHYSICS_STRUCTURE_TYPE_PHYSICS_SYSTEM);

        result = pSystem->initialize(pSimulation->resourcePool, pCollisionShapeLoader,
                                     rigidBodyPool, position3dPool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foePhysicsSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foePhysics, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues deinitializing after failed initialization", errors);
    }

    return result;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
}

} // namespace

int foePhysicsFunctionalityID() { return FOE_PHYSICS_LIBRARY_ID; }

extern "C" foeResultSet foePhysicsRegisterFunctionality() {
    FOE_LOG(foePhysics, FOE_LOG_LEVEL_VERBOSE,
            "foePhysicsRegisterFunctionality - Starting to register functionality")

    foeResultSet result = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foePhysicsFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
    });

    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                "foePhysicsRegisterFunctionality - Failed registering functionality: {}", buffer)
    } else {
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_VERBOSE,
                "foePhysicsRegisterFunctionality - Completed registering functionality")
    }

    return result;
}

extern "C" void foePhysicsDeregisterFunctionality() {
    FOE_LOG(foePhysics, FOE_LOG_LEVEL_VERBOSE,
            "foePhysicsDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foePhysicsFunctionalityID());

    FOE_LOG(foePhysics, FOE_LOG_LEVEL_VERBOSE,
            "foePhysicsDeregisterFunctionality - Completed deregistering functionality")
}