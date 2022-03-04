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

#define DESTROY_FUNCTIONALITY(X, Y, Z)                                                             \
    if ((pSelection == nullptr || pSelection->Z) && ptr->sType == Y && --ptr->refCount == 0) {     \
        delete (X *)ptr;                                                                           \
        ptr = nullptr;                                                                             \
        continue;                                                                                  \
    }

bool destroySelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    // Systems
    for (auto &ptr : pSimulationState->systems) {
        if (ptr == nullptr)
            continue;

        DESTROY_FUNCTIONALITY(VkAnimationPool, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL,
                              animationSystem);
        DESTROY_FUNCTIONALITY(PositionDescriptorPool,
                              FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL, positionSystem);
        DESTROY_FUNCTIONALITY(foeCameraSystem, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM,
                              cameraSystem);
        DESTROY_FUNCTIONALITY(foeArmatureSystem, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM,
                              armatureSystem);
    }

    // Components
    for (auto &ptr : pSimulationState->componentPools) {
        if (ptr == nullptr)
            continue;

        DESTROY_FUNCTIONALITY(foeRenderStatePool, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL,
                              renderStateComponents)
        DESTROY_FUNCTIONALITY(foeCameraPool, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL,
                              cameraComponents)
        DESTROY_FUNCTIONALITY(foeArmatureStatePool, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL,
                              armatureComponents)
    }

    return true;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    // Components
    if (auto *pPool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);
        pPool != nullptr) {
        ++pPool->refCount;
    } else {
        pPool = new foeArmatureStatePool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    if (auto *pPool = (foeCameraPool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);
        pPool != nullptr) {
        ++pPool->refCount;
    } else {
        pPool = new foeCameraPool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    if (auto *pPool = (foeRenderStatePool *)foeSimulationGetComponentPool(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);
        pPool != nullptr) {
        ++pPool->refCount;
    } else {
        pPool = new foeRenderStatePool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    // Systems
    foeSystemBase *pSystemBase = new foeArmatureSystem;
    ++pSystemBase->refCount;
    pSimulationState->systems.emplace_back(pSystemBase);

    pSystemBase = new foeCameraSystem;
    ++pSystemBase->refCount;
    pSimulationState->systems.emplace_back(pSystemBase);

    pSystemBase = new PositionDescriptorPool;
    ++pSystemBase->refCount;
    pSimulationState->systems.emplace_back(pSystemBase);

    pSystemBase = new VkAnimationPool;
    ++pSystemBase->refCount;
    pSimulationState->systems.emplace_back(pSystemBase);

    return {};
}

bool destroy(foeSimulationState *pSimulationState) {
    return destroySelection(pSimulationState, nullptr);
}

bool deinitializeSelection(foeSimulationState const *pSimulationState,
                           TypeSelection const *pSelection) {
    // Systems
    if (auto *pSystem = (foeArmatureSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
        (pSelection == nullptr || pSelection->armatureSystem) && --pSystem->initCount == 0) {
        pSystem->deinitialize();
    }

    if (auto *pSystem = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        (pSelection == nullptr || pSelection->cameraSystem) && --pSystem->initCount == 0) {
        pSystem->deinitialize();
    }

    if (auto *pSystem = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        (pSelection == nullptr || pSelection->positionSystem) && --pSystem->initCount == 0) {
        pSystem->deinitialize();
    }

    if (auto *pSystem = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        (pSelection == nullptr || pSelection->animationSystem) && --pSystem->initCount == 0) {
        pSystem->deinitialize();
    }

    return true;
}

auto initialize(foeSimulationState *pSimulationState, foeSimulationInitInfo const *pInitInfo)
    -> std::error_code {
    std::error_code errC;
    TypeSelection selection = {};

    // Systems
    if (auto *pArmatureSystem = (foeArmatureSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_SYSTEM);
        pArmatureSystem) {
        if (!pArmatureSystem->initialized()) {
            auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL);

            auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

            pArmatureSystem->initialize(pArmaturePool, pArmatureStatePool);
        }
        ++pArmatureSystem->initCount;
        selection.armatureSystem = true;
    }

    if (auto *pCameraSystem = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        pCameraSystem != nullptr) {
        if (!pCameraSystem->initialized()) {
            auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

            auto *pCameraPool = (foeCameraPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_POOL);

            errC = pCameraSystem->initialize(pPosition3dPool, pCameraPool);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pCameraSystem->initCount;
        selection.cameraSystem = true;
    }

    if (auto *pPositionDescriptorPool = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        pPositionDescriptorPool != nullptr) {
        if (!pPositionDescriptorPool->initialized()) {
            auto *pPosition3dPool = (foePosition3dPool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_POSITION_STRUCTURE_TYPE_POSITION_3D_POOL);

            errC = pPositionDescriptorPool->initialize(pPosition3dPool);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pPositionDescriptorPool->initCount;
        selection.positionSystem = true;
    }

    if (auto *pVkAnimationPool = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        pVkAnimationPool != nullptr) {
        if (!pVkAnimationPool->initialized()) {
            auto *pArmaturePool = (foeArmaturePool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL);
            auto *pMeshPool = (foeMeshPool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL);

            auto *pArmatureStatePool = (foeArmatureStatePool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_STATE_POOL);

            auto *pRenderStatePool = (foeRenderStatePool *)foeSimulationGetComponentPool(
                pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_RENDER_STATE_POOL);

            errC = pVkAnimationPool->initialize(pArmaturePool, pMeshPool, pArmatureStatePool,
                                                pRenderStatePool);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pVkAnimationPool->initCount;
        selection.animationSystem = true;
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitializeSelection(pSimulationState, &selection);

    return errC;
}

bool deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelection(pSimulationState, nullptr);
}

bool deinitializeGraphicsSelection(foeSimulationState const *pSimulationState,
                                   TypeSelection const *pSelection) {
    // Systems
    if (auto *pSystem = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        (pSelection == nullptr || pSelection->animationSystem) && --pSystem->gfxInitCount == 0) {
        pSystem->deinitializeGraphics();
    }

    if (auto *pSystem = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        (pSelection == nullptr || pSelection->positionSystem) && --pSystem->gfxInitCount == 0) {
        pSystem->deinitializeGraphics();
    }

    if (auto *pSystem = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        (pSelection == nullptr || pSelection->cameraSystem) && --pSystem->gfxInitCount == 0) {
        pSystem->deinitializeGraphics();
    }

    return true;
}

auto initializeGraphics(foeSimulationState *pSimulationState, foeGfxSession gfxSession)
    -> std::error_code {
    std::error_code errC;
    TypeSelection selection = {};

    // Systems
    if (auto *pCameraSystem = (foeCameraSystem *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_CAMERA_SYSTEM);
        pCameraSystem && !pCameraSystem->initializedGraphics()) {
        errC = pCameraSystem->initializeGraphics(gfxSession);
        if (errC)
            goto INITIALIZATION_FAILED;
        ++pCameraSystem->gfxInitCount;
        selection.cameraSystem = true;
    }

    if (auto *pPositionDescriptorPool = (PositionDescriptorPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_POSITION_DESCRIPTOR_POOL);
        pPositionDescriptorPool && !pPositionDescriptorPool->initializedGraphics()) {
        errC = pPositionDescriptorPool->initializeGraphics(gfxSession);
        if (errC)
            goto INITIALIZATION_FAILED;
        ++pPositionDescriptorPool->gfxInitCount;
        selection.positionSystem = true;
    }

    if (auto *pVkAnimationPool = (VkAnimationPool *)foeSimulationGetSystem(
            pSimulationState, FOE_BRINGUP_STRUCTURE_TYPE_VK_ANIMATION_POOL);
        pVkAnimationPool && !pVkAnimationPool->initializedGraphics()) {
        errC = pVkAnimationPool->initializeGraphics(gfxSession);
        if (errC)
            goto INITIALIZATION_FAILED;
        ++pVkAnimationPool->gfxInitCount;
        selection.animationSystem = true;
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitializeGraphicsSelection(pSimulationState, &selection);

    return errC;
}

bool deinitializeGraphics(foeSimulationState *pSimulationState) {
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