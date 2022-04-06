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

#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/position/component/3d_pool.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>
#include <vk_error_code.hpp>

#include "../error_code.hpp"
#include "../log.hpp"
#include "armature.hpp"
#include "armature_loader.hpp"
#include "armature_pool.hpp"
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
    // Resources
    bool armatureResources;
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

foeResourceCreateInfo importFn(void *pContext, foeResourceID resource) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);
    return pGroupData->getResourceDefinition(resource);
}

void armatureLoadFn(void *pContext, foeResource resource, PFN_foeResourcePostLoad *pPostLoadFn) {
    auto *pSimulation = reinterpret_cast<foeSimulation *>(pContext);

    auto createInfo = foeResourceGetCreateInfo(resource);

    for (auto const &it : pSimulation->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(createInfo)) {
            it.pLoadFn(it.pLoader, resource, createInfo, pPostLoadFn);
            return;
        }
    }

    pPostLoadFn(resource, foeToErrorCode(FOE_BRINGUP_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER),
                nullptr, nullptr, nullptr, nullptr, nullptr);
}

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        errC = foeSimulationDecrementRefCount(pSimulation,
                                              FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy VkAnimationPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            VkAnimationPool *pData;
            errC = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, (void **)&pData);
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
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy PositionDescriptorPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            PositionDescriptorPool *pData;
            errC = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, (void **)&pData);
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
        errC = foeSimulationDecrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                              &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeCameraSystem that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeCameraSystem *pData;
            errC = foeSimulationReleaseSystem(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                              (void **)&pData);
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
        errC = foeSimulationDecrementRefCount(pSimulation,
                                              FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureSystem that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeArmatureSystem *pData;
            errC = foeSimulationReleaseSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, (void **)&pData);
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
        errC = foeSimulationDecrementRefCount(pSimulation,
                                              FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeRenderStatePool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeRenderStatePool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL, (void **)&pData);
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
        errC = foeSimulationDecrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                              &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeCameraPool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeCameraPool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL, (void **)&pData);
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
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureStatePool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeArmatureStatePool *pData;
            errC = foeSimulationReleaseComponentPool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL, (void **)&pData);
            if (errC) {
                FOE_LOG(foeBringup, Warning,
                        "Could not release foeArmatureStatePool to destroy - {}", errC.message());
                ++errors;
            } else {
                delete pData;
            }
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        errC = foeSimulationDecrementRefCount(pSimulation,
                                              FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (errC) {
            // Trying to destroy something that doesn't exist? Not optimal
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmatureLoader that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeArmatureLoader *pLoader;
            errC = foeSimulationReleaseResourceLoader(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, (void **)&pLoader);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release foeArmatureLoader to destroy - {}",
                        errC.message());
                ++errors;
            } else {
                delete pLoader;
            }
        }
    }

    // Resources
    if (pSelection == nullptr || pSelection->armatureResources) {
        errC = foeSimulationDecrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL,
                                              &count);
        if (errC) {
            // Trying to destroy something that doesn't exist? Not optimal
            FOE_LOG(foeBringup, Warning,
                    "Attempted to decrement/destroy foeArmaturePool that doesn't exist - {}",
                    errC.message());
            ++errors;
        } else if (count == 0) {
            foeArmaturePool *pItem;
            errC = foeSimulationReleaseResourcePool(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL, (void **)&pItem);
            if (errC) {
                FOE_LOG(foeBringup, Warning, "Could not release foeArmaturePool to destroy - {}",
                        errC.message());
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
    TypeSelection created = {};

    // Resources
    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL,
                                       nullptr)) {
        foeSimulationResourcePoolData loaderCI{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL,
            .pResourcePool = new foeArmaturePool{foeResourceFns{
                .pImportContext = &pSimulation->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulation,
                .pLoadFn = armatureLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulation, &loaderCI);
        if (errC) {
            delete (foeArmatureLoader *)loaderCI.pResourcePool;
            FOE_LOG(foeBringup, Error,
                    "onCreate - Failed to create foeArmaturePool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL,
                                       nullptr);
    }
    created.armatureResources = true;

    // Loaders
    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
                                       nullptr)) {
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeArmatureLoader *)loaderCI.pLoader;
            FOE_LOG(foeBringup, Error,
                    "onCreate - Failed to create foeArmatureLoader on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
                                       nullptr);
    }
    created.armatureLoader = true;

    // Components
    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
            .pComponentPool = new foeArmatureStatePool,
            .pMaintenanceFn = [](void *pData) { ((foeArmatureStatePool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (errC) {
            delete (foeArmatureStatePool *)createInfo.pComponentPool;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeArmatureStatePool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
                                       nullptr);
    }
    created.armatureComponents = true;

    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
            .pComponentPool = new foeCameraPool,
            .pMaintenanceFn = [](void *pData) { ((foeCameraPool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (errC) {
            delete (foeCameraPool *)createInfo.pComponentPool;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeCameraPool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                                       nullptr);
    }
    created.cameraComponents = true;

    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
                                       nullptr)) {
        foeSimulationComponentPoolData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
            .pComponentPool = new foeRenderStatePool,
            .pMaintenanceFn = [](void *pData) { ((foeRenderStatePool *)pData)->maintenance(); },
        };
        errC = foeSimulationInsertComponentPool(pSimulation, &createInfo);
        if (errC) {
            delete (foeRenderStatePool *)createInfo.pComponentPool;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeRenderStatePool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
                                       nullptr);
    }
    created.renderStateComponents = true;

    // Systems
    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                                       nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
            .pSystem = new foeArmatureSystem,
        };
        errC = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (errC) {
            delete (foeArmatureSystem *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeArmatureSystem on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                                       nullptr);
    }
    created.armatureSystem = true;

    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                       nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
            .pSystem = new foeCameraSystem,
        };
        errC = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (errC) {
            delete (foeCameraSystem *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create foeCameraSystem on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                       nullptr);
    }
    created.cameraSystem = true;

    if (foeSimulationIncrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL,
            .pSystem = new PositionDescriptorPool,
        };
        errC = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (errC) {
            delete (PositionDescriptorPool *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create PositionDescriptorPool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, nullptr);
    }
    created.positionSystem = true;

    if (foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
                                       nullptr)) {
        foeSimulationSystemData createInfo{
            .sType = FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
            .pSystem = new VkAnimationPool,
        };
        errC = foeSimulationInsertSystem(pSimulation, &createInfo);
        if (errC) {
            delete (VkAnimationPool *)createInfo.pSystem;
            FOE_LOG(foeBringup, Error,
                    "create - Failed to create VkAnimationPool on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
                                       nullptr);
    }
    created.animationSystem = true;

CREATE_FAILED:
    if (errC) {
        size_t errors = destroySelection(pSimulation, &created);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning, "Encountered {} issues destroying after failed creation",
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
    if (pSelection == nullptr || pSelection->animationSystem) {
        errC = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement VkAnimationPool initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        errC = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (errC) {
            FOE_LOG(
                foeBringup, Warning,
                "Failed to decrement PositionDescriptorPool initialization count on Simulation {} "
                "with error {}",
                (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        errC = foeSimulationDecrementInitCount(pSimulation,
                                               FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeCameraSystem initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
            pData->deinitialize();
        }
    }

    if (pSelection == nullptr || pSelection->armatureSystem) {
        errC = foeSimulationDecrementInitCount(pSimulation,
                                               FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeArmatureSystem initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureSystem *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
            pLoader->deinitialize();
        }
    }

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        auto errC = foeSimulationDecrementInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeArmatureLoader initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER);
            pLoader->deinitialize();
        }
    }

    return errors;
}

auto initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo)
    -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Loaders
    errC = foeSimulationIncrementInitCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER,
                                           &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeArmatureLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.armatureLoader = true;
    if (count == 1) {
        auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_LOADER);
        errC = pLoader->initialize(pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeArmatureLoader on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    // Systems
    errC = foeSimulationIncrementInitCount((foeSimulation *)pSimulation,
                                           FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeArmatureSystem initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.armatureSystem = true;
    if (count == 1) {
        auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL);
        auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

        auto *pData = (foeArmatureSystem *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
        errC = pData->initialize(pArmaturePool, pArmatureStatePool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeArmatureSystem on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount((foeSimulation *)pSimulation,
                                           FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeCameraSystem initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
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
        errC = pData->initialize(pPosition3dPool, pCameraPool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize foeCameraSystem on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        (foeSimulation *)pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
    if (errC) {
        FOE_LOG(
            foeBringup, Error,
            "Failed to increment PositionDescriptorPool initialization count on Simulation {} with "
            "error {}",
            (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.positionSystem = true;
    if (count == 1) {
        auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
            pSimulation, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

        auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        errC = pData->initialize(pPosition3dPool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize PositionDescriptorPool on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount((foeSimulation *)pSimulation,
                                           FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment VkAnimationPool initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.animationSystem = true;
    if (count == 1) {
        auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_POOL);
        auto *pMeshPool = (foeMeshPool *)foeSimulationGetResourcePool(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);
        auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
        auto *pRenderStatePool = (foeRenderStatePool *)foeSimulationGetComponentPool(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);

        auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        errC = pData->initialize(pArmaturePool, pMeshPool, pArmatureStatePool, pRenderStatePool);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize VkAnimationPool on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning,
                    "Encountered {} issues deinitializing after failed initialization", errors);
    }

    return errC;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
}

size_t deinitializeGraphicsSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    size_t errors = 0;

    // Systems
    if (pSelection == nullptr || pSelection->animationSystem) {
        errC = foeSimulationDecrementGfxInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement VkAnimationPool graphics initialization count on "
                    "Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
            pData->deinitializeGraphics();
        }
    }

    if (pSelection == nullptr || pSelection->positionSystem) {
        errC = foeSimulationDecrementGfxInitCount(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement PositionDescriptorPool graphics initialization count on "
                    "Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
            pData->deinitializeGraphics();
        }
    }

    if (pSelection == nullptr || pSelection->cameraSystem) {
        errC = foeSimulationDecrementGfxInitCount(pSimulation,
                                                  FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM, &count);
        if (errC) {
            FOE_LOG(foeBringup, Warning,
                    "Failed to decrement foeCameraSystem graphics initialization count on "
                    "Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            ++errors;
        } else if (count == 0) {
            auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
                pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
            pData->deinitializeGraphics();
        }
    }

    return errors;
}

auto initializeGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) -> std::error_code {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Systems
    errC = foeSimulationIncrementGfxInitCount(pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                                              &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment foeCameraSystem graphics initialization count on Simulation "
                "{} with error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.cameraSystem = true;
    if (count == 1) {
        auto *pData = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        errC = pData->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics foeCameraSystem on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementGfxInitCount(
        pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment PositionDescriptorPool graphics initialization count on "
                "Simulation {} with error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.positionSystem = true;
    if (count == 1) {
        auto *pData = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        errC = pData->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics PositionDescriptorPool on Simulation {} with "
                    "error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementGfxInitCount(pSimulation,
                                              FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL, &count);
    if (errC) {
        FOE_LOG(foeBringup, Error,
                "Failed to increment VkAnimationPool graphics initialization count on "
                "Simulation {} with error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.animationSystem = true;
    if (count == 1) {
        auto *pData = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulation, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        errC = pData->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeBringup, Error,
                    "Failed to initialize graphics VkAnimationPool on Simulation {} with "
                    "error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeGraphicsSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeBringup, Warning,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return errC;
}

size_t deinitializeGraphics(foeSimulation *pSimulation) {
    return deinitializeGraphicsSelection(pSimulation, nullptr);
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