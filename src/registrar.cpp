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

#include "registrar.hpp"

#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/type_defs.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>
#include <vk_error_code.hpp>

#include "armature_state_pool.hpp"
#include "armature_system.hpp"
#include "camera_pool.hpp"
#include "camera_system.hpp"
#include "log.hpp"
#include "position_descriptor_pool.hpp"
#include "render_state_pool.hpp"
#include "type_defs.h"
#include "vk_animation.hpp"

namespace {

struct TypeSelection {
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

size_t destroySelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy VkAnimationPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            VkAnimationPool *pData;
            errC = foeSimulationReleaseSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release VkAnimationPool to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        errC = foeSimulationDecrementRefCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy PositionDescriptorPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            PositionDescriptorPool *pData;
            errC = foeSimulationReleaseSystem(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL,
                                              (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning,
                        "Could not release PositionDescriptorPool to destroy - {}", errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeCameraSystem that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeCameraSystem *pData;
            errC = foeSimulationReleaseSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release foeCameraSystem to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->armatureSystem) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureSystem that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeArmatureSystem *pData;
            errC = foeSimulationReleaseSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release foeArmatureSystem to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Components
    if (pSelection == nullptr || pSelection->renderStateComponents) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeRenderStatePool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeRenderStatePool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release foeRenderStatePool to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->cameraComponents) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeCameraPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeCameraPool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release foeCameraPool to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    if (pSelection == nullptr || pSelection->armatureComponents) {
        errC = foeSimulationDecrementRefCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureStatePool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeArmatureStatePool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning,
                        "Could not release foeArmatureStatePool to destroy - {}", errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    return errors;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    std::error_code errC;
    TypeSelection created = {};

    // Components
    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
            .pComponentPool = new foeArmatureStatePool,
            .pMaintenanceFn = [](void *pData) { ((foeArmatureStatePool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeArmatureStatePool *)createInfo.pComponentPool;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeArmatureStatePool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, nullptr);
    }
    created.armatureComponents = true;

    if (foeSimulationIncrementRefCount(pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
            .pComponentPool = new foeCameraPool,
            .pMaintenanceFn = [](void *pData) { ((foeCameraPool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeCameraPool *)createInfo.pComponentPool;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeCameraPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                       nullptr);
    }
    created.cameraComponents = true;

    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
            .pComponentPool = new foeRenderStatePool,
            .pMaintenanceFn = [](void *pData) { ((foeRenderStatePool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeRenderStatePool *)createInfo.pComponentPool;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeRenderStatePool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, nullptr);
    }
    created.renderStateComponents = true;

    // Systems
    if (foeSimulationIncrementRefCount(pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                                       nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
            .pSystem = new foeArmatureSystem,
        };
        errC = foeSimulationInsertSystem(pSimulationState, &createInfo);
        if (errC) {
            delete (foeArmatureSystem *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeArmatureSystem on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                                       nullptr);
    }
    created.armatureSystem = true;

    if (foeSimulationIncrementRefCount(pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                       nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
            .pSystem = new foeCameraSystem,
        };
        errC = foeSimulationInsertSystem(pSimulationState, &createInfo);
        if (errC) {
            delete (foeCameraSystem *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeCameraSystem on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                       nullptr);
    }
    created.cameraSystem = true;

    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL,
            .pSystem = new PositionDescriptorPool,
        };
        errC = foeSimulationInsertSystem(pSimulationState, &createInfo);
        if (errC) {
            delete (PositionDescriptorPool *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create PositionDescriptorPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, nullptr);
    }
    created.positionSystem = true;

    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
            .pSystem = new VkAnimationPool,
        };
        errC = foeSimulationInsertSystem(pSimulationState, &createInfo);
        if (errC) {
            delete (VkAnimationPool *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create VkAnimationPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, nullptr);
    }
    created.animationSystem = true;

CREATE_FAILED:
    if (errC) {
        size_t errors = destroySelection(pSimulationState, &created);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning, "Encountered {} issues destroying after failed creation",
                    errors);
    }

    return errC;
}

size_t destroy(foeSimulationState *pSimulationState) {
    return destroySelection(pSimulationState, nullptr);
}

size_t deinitializeSelection(foeSimulationState *pSimulationState,
                             TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        errC = foeSimulationDecrementInitCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement VkAnimationPool initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        errC = foeSimulationDecrementInitCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (errC) {
            FOE_LOG(
                foeBringup, Warning,
                "Failed to decrement PositionDescriptorPool initialization count on Simulation {} "
                "with error {}",
                (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        errC = foeSimulationDecrementInitCount(pSimulationState,
                                               FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeCameraSystem initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->armatureSystem) {
        errC = foeSimulationDecrementInitCount(pSimulationState,
                                               FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeArmatureSystem initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureSystem *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
            pLoader->deinitialize();
        }
    }

    return errors;
}

auto initialize(foeSimulationState *pSimulationState, foeSimulationInitInfo const *pInitInfo)
    -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Systems
    errC = foeSimulationIncrementInitCount((foeSimulationState *)pSimulationState,
                                           FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeArmatureSystem initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL);

        auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

        auto *pData = (foeArmatureSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
        errC = pData->initialize(pArmaturePool, pArmatureStatePool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeArmatureSystem on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }
    selection.armatureSystem = true;

    errC = foeSimulationIncrementInitCount((foeSimulationState *)pSimulationState,
                                           FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeCameraSystem initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto *pCameraPool = (foeCameraPool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);

        auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        errC = pData->initialize(pPosition3dPool, pCameraPool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeCameraSystem on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }
    selection.cameraSystem = true;

    errC = foeSimulationIncrementInitCount((foeSimulationState *)pSimulationState,
                                           FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL,
                                           &count);
    if (errC) {
        FOE_LOG(
            foeBringup, Error,
            "Failed to increment PositionDescriptorPool initialization count on Simulation {} with "
            "error {}",
            (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        errC = pData->initialize(pPosition3dPool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize PositionDescriptorPool on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }
    selection.positionSystem = true;

    errC = foeSimulationIncrementInitCount((foeSimulationState *)pSimulationState,
                                           FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment VkAnimationPool initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL);
        auto *pMeshPool = (foeMeshPool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

        auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

        auto *pRenderStatePool = (foeRenderStatePool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);

        auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        errC = pData->initialize(pArmaturePool, pMeshPool, pArmatureStatePool, pRenderStatePool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize VkAnimationPool on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }
    selection.animationSystem = true;

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeSelection(pSimulationState, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning,
                    "Encountered {} issues deinitializing after failed initialization", errors);
    }

    return errC;
}

size_t deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelection(pSimulationState, nullptr);
}

size_t deinitializeGraphicsSelection(foeSimulationState *pSimulationState,
                                   TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        errC = foeSimulationDecrementGfxInitCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement VkAnimationPool graphics initialization count on "
                    "Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
            pData->deinitializeGraphics();
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        errC = foeSimulationDecrementGfxInitCount(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement PositionDescriptorPool graphics initialization count on "
                    "Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
            pData->deinitializeGraphics();
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        errC = foeSimulationDecrementGfxInitCount(pSimulationState,
                                                  FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeCameraSystem graphics initialization count on "
                    "Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
            pData->deinitializeGraphics();
        }
    }

    return errors;
}

auto initializeGraphics(foeSimulationState *pSimulationState, foeGfxSession gfxSession)
    -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Systems
    errC = foeSimulationIncrementGfxInitCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
    selection.cameraSystem = true;
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeCameraSystem graphics initialization count on Simulation "
                "{} with error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        errC = pData->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics foeCameraSystem on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementGfxInitCount(
        pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
    selection.positionSystem = true;
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment PositionDescriptorPool graphics initialization count on "
                "Simulation {} with error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        errC = pData->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics PositionDescriptorPool on Simulation {} with "
                    "error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementGfxInitCount(pSimulationState,
                                              FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
    selection.animationSystem = true;
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment VkAnimationPool graphics initialization count on "
                "Simulation {} with error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        errC = pData->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics VkAnimationPool on Simulation {} with "
                    "error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeGraphicsSelection(pSimulationState, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return errC;
}

size_t deinitializeGraphics(foeSimulationState *pSimulationState) {
    return deinitializeGraphicsSelection(pSimulationState, nullptr);
}

} // namespace

auto foeBringupRegisterFunctionality() -> std::error_code {
    FOE_LOG(foeBringup, Verbose,
            "foeBringupRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = FOE_BRINGUP_APP_FUNCTIONALITY_ID,
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
        .pInitializeGraphicsFn = initializeGraphics,
        .pDeinitializeGraphicsFn = deinitializeGraphics,
    });

    if (errC) {
        FOE_LOG(foeBringup, Error,
                "foeBringupRegisterFunctionality - Failed registering functionality: {} - {}",
                errC.value(), errC.message())
    } else {
        FOE_LOG(foeBringup, Verbose,
                "foeBringupRegisterFunctionality - Completed registering functionality")
    }

    return errC;
}

void foeBringupDeregisterFunctionality() {
    FOE_LOG(foeBringup, Verbose,
            "foeBringupDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(FOE_BRINGUP_APP_FUNCTIONALITY_ID);

    FOE_LOG(foeBringup, Verbose,
            "foeBringupDeregisterFunctionality - Completed deregistering functionality")
}