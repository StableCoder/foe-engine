// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "registration.h"

#include <foe/graphics/resource/type_defs.h>
#include <foe/position/component/3d_pool.h>
#include <foe/position/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource_fns.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "../log.hpp"
#include "../result.h"
#include "animated_bone_state.hpp"
#include "animated_bone_state_pool.h"
#include "animated_bone_system.h"
#include "armature_loader.hpp"
#include "armature_state.h"
#include "armature_state_pool.h"
#include "render_state_pool.h"
#include "render_system.hpp"
#include "type_defs.h"

namespace {

struct TypeSelection {
    // Loaders
    bool armatureLoader;
    // Components
    bool armatureComponents;
    bool animatedBoneStateComponents;
    bool renderStateComponents;
    // Systems
    bool animatedBoneSystem;
    bool renderSystem;
};

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t count;
    size_t errors = 0;
    foeResultSet result;

    // Systems
    if (pSelection == nullptr || pSelection->renderSystem) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeRenderSystem that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeRenderSystem renderSystem;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM, (void **)&renderSystem);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeRenderSystem to destroy: {}", buffer);

                ++errors;
            } else {
                foeDestroyRenderSystem(renderSystem);
            }
        }
    }

    if (pSelection == nullptr || pSelection->animatedBoneSystem) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeAnimatedBoneSystem that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeAnimatedBoneSystem system;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM, (void **)&system);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeAnimatedBoneSystem to destroy: {}", buffer);

                ++errors;
            } else {
                foeDestroyAnimatedBoneSystem((foeAnimatedBoneSystem)system);
            }
        }
    }

    // Components
    if (pSelection == nullptr || pSelection->renderStateComponents) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeRenderStatePool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeRenderStatePool pool;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL, (void **)&pool);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeRenderStatePool to destroy: {}", buffer);

                ++errors;
            } else {
                foeEcsDestroyComponentPool(pool);
            }
        }
    }

    if (pSelection == nullptr || pSelection->animatedBoneStateComponents) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(
                foeBringup, FOE_LOG_LEVEL_WARNING,
                "Attempted to decrement/destroy foeAnimatedBoneStatePool that doesn't exist: {}",
                buffer);

            ++errors;
        } else if (count == 0) {
            foeAnimatedBoneStatePool componentPool;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL,
                (void **)&componentPool);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeAnimatedBoneStatePool to destroy: {}", buffer);

                ++errors;
            } else {
                foeEcsDestroyComponentPool(componentPool);
            }
        }
    }

    if (pSelection == nullptr || pSelection->armatureComponents) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeArmatureStatePool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeArmatureStatePool componentPool;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
                (void **)&componentPool);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeArmatureStatePool to destroy: {}", buffer);

                ++errors;
            } else {
                foeEcsDestroyComponentPool(componentPool);
            }
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (result.value != FOE_SUCCESS) {
            // Trying to destroy something that doesn't exist? Not optimal
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Attempted to decrement/destroy foeArmatureLoader that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeArmatureLoader *pLoader;
            result = foeSimulationReleaseResourceLoader(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER, (void **)&pLoader);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                        "Could not release foeArmatureLoader to destroy: {}", buffer);

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
    TypeSelection created = {};

    // Loaders
    result = foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER,
            .pLoader = new (std::nothrow) foeArmatureLoader,
            .pCanProcessCreateInfoFn = foeArmatureLoader::canProcessCreateInfo,
            .pLoadFn = foeArmatureLoader::load,
            .pMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeArmatureLoader *>(pLoader)->maintenance();
                },
        };
        if (loaderCI.pLoader == nullptr) {
            result = to_foeResult(FOE_SKUNKWORKS_ERROR_OUT_OF_MEMORY);
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (result.value != FOE_SUCCESS) {
            delete (foeArmatureLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeArmatureLoader on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER,
                                       nullptr);
    }
    created.armatureLoader = true;

    // Components
    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
            .pMaintenanceFn =
                [](void *pData) { foeEcsComponentPoolMaintenance((foeEcsComponentPool)pData); },
        };

        result = foeEcsCreateComponentPool(0, 16, sizeof(foeArmatureState), nullptr,
                                           (foeEcsComponentPool *)&createInfo.pComponentPool);
        if (result.value != FOE_NULL_HANDLE) {
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeArmatureStatePool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "create - Failed to create foeArmatureStatePool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
                                       nullptr);
    }
    created.armatureComponents = true;

    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL,
            .pMaintenanceFn =
                [](void *componentPool) {
                    foeEcsComponentPoolMaintenance((foeAnimatedBoneStatePool)componentPool);
                },
        };

        result =
            foeEcsCreateComponentPool(0, 16, sizeof(foeAnimatedBoneState),
                                      (PFN_foeEcsComponentDestructor)cleanup_foeAnimatedBoneState,
                                      (foeEcsComponentPool *)&createInfo.pComponentPool);
        if (result.value != FOE_NULL_HANDLE) {
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeAnimatedBoneStatePool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "create - Failed to create foeAnimatedBoneStatePool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL, nullptr);
    }
    created.animatedBoneStateComponents = true;

    result = foeSimulationIncrementRefCount(pSimulation,
                                            FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL,
            .pMaintenanceFn =
                [](void *componentPool) {
                    foeEcsComponentPoolMaintenance((foeRenderStatePool)componentPool);
                },
        };

        result = foeEcsCreateComponentPool(0, 16, sizeof(foeRenderState), nullptr,
                                           (foeEcsComponentPool *)&createInfo.pComponentPool);
        if (result.value != FOE_SUCCESS) {
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeRenderStatePool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "create - Failed to create foeRenderStatePool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL,
                                       nullptr);
    }
    created.renderStateComponents = true;

    // Systems
    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM,
        };

        result = foeCreateAnimatedBoneSystem((foeAnimatedBoneSystem *)&createInfo.pSystem);
        if (result.value != FOE_SUCCESS) {
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            foeDestroyAnimatedBoneSystem((foeAnimatedBoneSystem)createInfo.pSystem);

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "create - Failed to create foeArmatureSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM,
                                       nullptr);
    }
    created.animatedBoneSystem = true;

    result = foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM,
        };

        result = foeCreateRenderSystem((foeRenderSystem *)&createInfo.pSystem);
        if (result.value != FOE_SUCCESS) {
            goto CREATE_FAILED;
        }

        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            foeDestroyRenderSystem((foeRenderSystem)createInfo.pSystem);

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "create - Failed to create foeRenderSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM,
                                       nullptr);
    }
    created.renderSystem = true;

CREATE_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = destroySelection(pSimulation, &created);
        if (errors > 0)
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues destroying after failed creation", errors);
    }

    return result;
}

size_t destroy(foeSimulation *pSimulation) { return destroySelection(pSimulation, nullptr); }

size_t deinitializeSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    foeResultSet result;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animatedBoneSystem) {
        result = foeSimulationDecrementInitCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Failed to decrement foeAnimatedBoneSystem initialization count on Simulation "
                    "{}: {}",
                    (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            foeAnimatedBoneSystem system = (foeAnimatedBoneSystem)foeSimulationGetSystem(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM);
            foeDeinitializeAnimatedBoneSystem(system);
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        auto result = foeSimulationDecrementInitCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(
                foeBringup, FOE_LOG_LEVEL_WARNING,
                "Failed to decrement foeArmatureLoader initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER);
            pLoader->deinitialize();
        }
    }

    return errors;
}

foeResultSet initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo) {
    foeResultSet result;
    size_t count;
    TypeSelection selection = {};

    // Loaders
    result = foeSimulationIncrementInitCount(pSimulation,
                                             FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeArmatureLoader initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.armatureLoader = true;
    if (count == 1) {
        auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeArmatureLoader on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    result = foeSimulationIncrementInitCount(
        (foeSimulation *)pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeArmatureSystem initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.animatedBoneSystem = true;
    if (count == 1) {
        foeArmatureStatePool armatureStatePool =
            (foeArmatureStatePool)foeSimulationGetComponentPool(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
        foeAnimatedBoneStatePool animatedBoneStatePool =
            (foeAnimatedBoneStatePool)foeSimulationGetComponentPool(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL);

        foeAnimatedBoneSystem system = (foeAnimatedBoneSystem)foeSimulationGetSystem(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_SYSTEM);
        result = foeInitializeAnimatedBoneSystem(system, pSimulation->resourcePool,
                                                 armatureStatePool, animatedBoneStatePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeAnimatedBoneSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues deinitializing after failed initialization", errors);
    }

    return result;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
}

size_t deinitializeGraphicsSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    foeResultSet result;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->renderSystem) {
        result = foeSimulationDecrementGfxInitCount(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Failed to decrement foeRenderSystem graphics initialization count on "
                    "Simulation {}: {}",
                    (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            foeRenderSystem renderSystem = (foeRenderSystem)foeSimulationGetSystem(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM);
            foeDeinitializeRenderSystemGraphics(renderSystem);
        }
    }

    return errors;
}

foeResultSet initializeGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) {
    foeResultSet result;
    size_t count;
    TypeSelection selection = {};

    // Systems
    result = foeSimulationIncrementGfxInitCount(pSimulation,
                                                FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeRenderSystem graphics initialization count on "
                "Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.renderSystem = true;
    if (count == 1) {
        foeRenderSystem renderSystem = (foeRenderSystem)foeSimulationGetSystem(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_SYSTEM);

        foeRenderStatePool renderStatePool = (foeRenderStatePool)foeSimulationGetComponentPool(
            pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_RENDER_STATE_POOL);
        foePosition3dPool positionPool = (foeRenderStatePool)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
        foeAnimatedBoneStatePool animatedBoneStatePool =
            (foeAnimatedBoneStatePool)foeSimulationGetComponentPool(
                pSimulation, FOE_SKUNKWORKS_STRUCTURE_TYPE_ANIMATED_BONE_STATE_POOL);

        result =
            foeInitializeRenderSystemGraphics(renderSystem, gfxSession, pSimulation->resourcePool,
                                              renderStatePool, positionPool, animatedBoneStatePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize graphics foeRenderSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeGraphicsSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return result;
}

size_t deinitializeGraphics(foeSimulation *pSimulation) {
    return deinitializeGraphicsSelection(pSimulation, nullptr);
}

} // namespace

foeResultSet foeBringupRegisterFunctionality() {
    FOE_LOG(foeBringup, FOE_LOG_LEVEL_VERBOSE,
            "foeBringupRegisterFunctionality - Starting to register functionality")

    foeResultSet result = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = 0,
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
        .pInitializeGraphicsFn = initializeGraphics,
        .pDeinitializeGraphicsFn = deinitializeGraphics,
    });

    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_ERROR,
                "foeBringupRegisterFunctionality - Failed registering functionality: {}", buffer)
    } else {
        FOE_LOG(foeBringup, FOE_LOG_LEVEL_VERBOSE,
                "foeBringupRegisterFunctionality - Completed registering functionality")
    }

    return result;
}

void foeBringupDeregisterFunctionality() {
    FOE_LOG(foeBringup, FOE_LOG_LEVEL_VERBOSE,
            "foeBringupDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(0);

    FOE_LOG(foeBringup, FOE_LOG_LEVEL_VERBOSE,
            "foeBringupDeregisterFunctionality - Completed deregistering functionality")
}