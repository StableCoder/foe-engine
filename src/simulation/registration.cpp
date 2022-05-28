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

#include "registration.hpp"

#include <foe/graphics/resource/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource_fns.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "../log.hpp"
#include "../result.h"
#include "armature.hpp"
#include "armature_loader.hpp"
#include "armature_state_pool.hpp"
#include "armature_system.hpp"
#include "camera_pool.hpp"
#include "camera_system.hpp"
#include "position_descriptor_pool.hpp"
#include "render_state_pool.hpp"
#include "type_defs.h"
#include "vk_animation.hpp"

namespace {

struct TypeSelection {
    // Loaders
    bool armatureLoader;
    // Components
    bool armatureComponents;
    bool cameraComponents;
    bool renderStateComponents;
    // Systems
    bool armatureSystem;
    bool cameraSystem;
    bool positionSystem;
    bool animationSystem;
};

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t count;
    size_t errors = 0;
    foeResult result;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy VkAnimationPool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            VkAnimationPool *pData;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning, "Could not release VkAnimationPool to destroy: {}",
                        buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy PositionDescriptorPool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            PositionDescriptorPool *pData;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning,
                        "Could not release PositionDescriptorPool to destroy: {}", buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeCameraSystem that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeCameraSystem *pData;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning, "Could not release foeCameraSystem to destroy: {}",
                        buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->armatureSystem) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureSystem that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeArmatureSystem *pData;
            result = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning, "Could not release foeArmatureSystem to destroy: {}",
                        buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Components
    if (pSelection == nullptr || pSelection->renderStateComponents) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeRenderStatePool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeRenderStatePool *pData;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning, "Could not release foeRenderStatePool to destroy: {}",
                        buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->cameraComponents) {
        result = foeSimulationDecrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                                &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeCameraPool that doesn't exist: {}", buffer);

            ++errors;
        } else if (count == 0) {
            foeCameraPool *pData;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning, "Could not release foeCameraPool to destroy: {}",
                        buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->armatureComponents) {
        result = foeSimulationDecrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureStatePool that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeArmatureStatePool *pData;
            result = foeSimulationReleaseComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, (void **)&pData);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning,
                        "Could not release foeArmatureStatePool to destroy: {}", buffer);

                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        result = foeSimulationDecrementRefCount(pSimulation,
                                                FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (result.value != FOE_SUCCESS) {
            // Trying to destroy something that doesn't exist? Not optimal
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureLoader that doesn't exist: {}",
                    buffer);

            ++errors;
        } else if (count == 0) {
            foeArmatureLoader *pLoader;
            result = foeSimulationReleaseResourceLoader(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, (void **)&pLoader);
            if (result.value != FOE_SUCCESS) {
                char buffer[FOE_MAX_RESULT_STRING_SIZE];
                result.toString(result.value, buffer);
                FOE_LOG(foeBringup, Warning, "Could not release foeArmatureLoader to destroy: {}",
                        buffer);

                ++errors;
            } else {
                delete pLoader;
            }
        }
    }

    return errors;
}

foeResult create(foeSimulation *pSimulation) {
    foeResult result;
    TypeSelection created = {};

    // Loaders
    result = foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
            .pLoader = new foeArmatureLoader,
            .pCanProcessCreateInfoFn = foeArmatureLoader::canProcessCreateInfo,
            .pLoadFn = foeArmatureLoader::load,
            .pMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeArmatureLoader *>(pLoader)->maintenance();
                },
        };
        result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (result.value != FOE_SUCCESS) {
            delete (foeArmatureLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "onCreate - Failed to create foeArmatureLoader on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
                                       nullptr);
    }
    created.armatureLoader = true;

    // Components
    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
            .pComponentPool = new foeArmatureStatePool,
            .pMaintenanceFn = [](void *pData) { ((foeArmatureStatePool *)pData)->maintenance(); },
        };
        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeArmatureStatePool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeArmatureStatePool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
                                       nullptr);
    }
    created.armatureComponents = true;

    result = foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
            .pComponentPool = new foeCameraPool,
            .pMaintenanceFn = [](void *pData) { ((foeCameraPool *)pData)->maintenance(); },
        };
        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeCameraPool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeCameraPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                       nullptr);
    }
    created.cameraComponents = true;

    result = foeSimulationIncrementRefCount(pSimulation,
                                            FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
            .pComponentPool = new foeRenderStatePool,
            .pMaintenanceFn = [](void *pData) { ((foeRenderStatePool *)pData)->maintenance(); },
        };
        result = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeRenderStatePool *)createInfo.pComponentPool;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeRenderStatePool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
                                       nullptr);
    }
    created.renderStateComponents = true;

    // Systems
    result = foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
            .pSystem = new foeArmatureSystem,
        };
        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeArmatureSystem *)createInfo.pSystem;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeArmatureSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                                       nullptr);
    }
    created.armatureSystem = true;

    result = foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                            nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
            .pSystem = new foeCameraSystem,
        };
        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (foeCameraSystem *)createInfo.pSystem;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeCameraSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                       nullptr);
    }
    created.cameraSystem = true;

    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL,
            .pSystem = new PositionDescriptorPool,
        };
        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (PositionDescriptorPool *)createInfo.pSystem;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create PositionDescriptorPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, nullptr);
    }
    created.positionSystem = true;

    result = foeSimulationIncrementRefCount(pSimulation,
                                            FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, nullptr);
    if (result.value != FOE_SUCCESS) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
            .pSystem = new VkAnimationPool,
        };
        result = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (result.value != FOE_SUCCESS) {
            delete (VkAnimationPool *)createInfo.pSystem;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create VkAnimationPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
                                       nullptr);
    }
    created.animationSystem = true;

CREATE_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = destroySelection(pSimulation, &created);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning, "Encountered {} issues destroying after failed creation",
                    errors);
    }

    return result;
}

size_t destroy(foeSimulation *pSimulation) { return destroySelection(pSimulation, nullptr); }

size_t deinitializeSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    foeResult result;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        result = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement VkAnimationPool initialization count on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            ++errors;
        } else if (count == 0) {
            auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        result = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement PositionDescriptorPool initialization count on Simulation "
                    "{}: {}",
                    (void *)pSimulation, buffer);

            ++errors;
        } else if (count == 0) {
            auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        result = foeSimulationDecrementInitCount(pSimulation,
                                                 FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeCameraSystem initialization count on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            ++errors;
        } else if (count == 0) {
            auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->armatureSystem) {
        result = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(
                foeBringup, Warning,
                "Failed to decrement foeArmatureSystem initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureSystem *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
            pLoader->deinitialize();
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        auto result = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(
                foeBringup, Warning,
                "Failed to decrement foeArmatureLoader initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER);
            pLoader->deinitialize();
        }
    }

    return errors;
}

foeResult initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo) {
    foeResult result;
    size_t count;
    TypeSelection selection = {};

    // Loaders
    result = foeSimulationIncrementInitCount(pSimulation,
                                             FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeArmatureLoader initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.armatureLoader = true;
    if (count == 1) {
        auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeArmatureLoader on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    result = foeSimulationIncrementInitCount((foeSimulation *)pSimulation,
                                             FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeArmatureSystem initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.armatureSystem = true;
    if (count == 1) {
        auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

        auto *pData = (foeArmatureSystem *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
        result = pData->initialize(pSimulation->resourcePool, pArmatureStatePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeArmatureSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount((foeSimulation *)pSimulation,
                                             FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeCameraSystem initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.cameraSystem = true;
    if (count == 1) {
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);
        auto *pCameraPool = (foeCameraPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);

        auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        result = pData->initialize(pPosition3dPool, pCameraPool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error, "Failed to initialize foeCameraSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount(
        (foeSimulation *)pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(
            foeBringup, Error,
            "Failed to increment PositionDescriptorPool initialization count on Simulation {}: {}",
            (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.positionSystem = true;
    if (count == 1) {
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        result = pData->initialize(pPosition3dPool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize PositionDescriptorPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount((foeSimulation *)pSimulation,
                                             FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment VkAnimationPool initialization count on Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.animationSystem = true;
    if (count == 1) {
        auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
        auto *pRenderStatePool = (foeRenderStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);

        auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);

        result = pData->initialize(pSimulation->resourcePool, pArmatureStatePool, pRenderStatePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error, "Failed to initialize VkAnimationPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning,
                    "Encountered {} issues deinitializing after failed initialization", errors);
    }

    return result;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
}

size_t deinitializeGraphicsSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    foeResult result;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        result = foeSimulationDecrementGfxInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement VkAnimationPool graphics initialization count on "
                    "Simulation {}: {}",
                    (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
            pData->deinitializeGraphics();
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        result = foeSimulationDecrementGfxInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement PositionDescriptorPool graphics initialization count on "
                    "Simulation {}: {}",
                    (void *)pSimulation, buffer);
            ++errors;
        } else if (count == 0) {
            auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
            pData->deinitializeGraphics();
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        result = foeSimulationDecrementGfxInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeCameraSystem graphics initialization count on "
                    "Simulation {}: {}",
                    (void *)pSimulation, buffer);

            ++errors;
        } else if (count == 0) {
            auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
            pData->deinitializeGraphics();
        }
    }

    return errors;
}

foeResult initializeGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) {
    foeResult result;
    size_t count;
    TypeSelection selection = {};

    // Systems
    result = foeSimulationIncrementGfxInitCount(pSimulation,
                                                FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeCameraSystem graphics initialization count on Simulation "
                "{}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.cameraSystem = true;
    if (count == 1) {
        auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        result = pData->initializeGraphics(gfxSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics foeCameraSystem on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementGfxInitCount(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment PositionDescriptorPool graphics initialization count on "
                "Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.positionSystem = true;
    if (count == 1) {
        auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        result = pData->initializeGraphics(gfxSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics PositionDescriptorPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementGfxInitCount(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeBringup, Error,
                "Failed to increment VkAnimationPool graphics initialization count on "
                "Simulation {}: {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.animationSystem = true;
    if (count == 1) {
        auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        result = pData->initializeGraphics(gfxSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics VkAnimationPool on Simulation {}: {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeGraphicsSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return result;
}

size_t deinitializeGraphics(foeSimulation *pSimulation) {
    return deinitializeGraphicsSelection(pSimulation, nullptr);
}

} // namespace

foeResult foeBringupRegisterFunctionality() {
    FOE_LOG(foeBringup, Verbose,
            "foeBringupRegisterFunctionality - Starting to register functionality")

    foeResult result = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = FOE_BRINGUP_APP_FUNCTIONALITY_ID,
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
        FOE_LOG(foeBringup, Error,
                "foeBringupRegisterFunctionality - Failed registering functionality: {}", buffer)
    } else {
        FOE_LOG(foeBringup, Verbose,
                "foeBringupRegisterFunctionality - Completed registering functionality")
    }

    return result;
}

void foeBringupDeregisterFunctionality() {
    FOE_LOG(foeBringup, Verbose,
            "foeBringupDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(FOE_BRINGUP_APP_FUNCTIONALITY_ID);

    FOE_LOG(foeBringup, Verbose,
            "foeBringupDeregisterFunctionality - Completed deregistering functionality")
}