/*
    Copyright (C) 2021 George Cave.

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
#include <foe/position/component/3d_pool.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/registration_fn_templates.hpp>
#include <foe/simulation/simulation.hpp>
#include <vk_error_code.hpp>

#include "armature_state_pool.hpp"
#include "armature_system.hpp"
#include "camera_pool.hpp"
#include "camera_system.hpp"
#include "log.hpp"
#include "position_descriptor_pool.hpp"
#include "render_state_pool.hpp"
#include "vk_animation.hpp"

namespace {

void onCreate(foeSimulationState *pSimulationState) {
    // Components
    if (auto *pPool = search<foeArmatureStatePool>(pSimulationState->componentPools.begin(),
                                                   pSimulationState->componentPools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeArmatureStatePool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    if (auto *pPool = search<foeCameraPool>(pSimulationState->componentPools.begin(),
                                            pSimulationState->componentPools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeCameraPool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    if (auto *pPool = search<foeRenderStatePool>(pSimulationState->componentPools.begin(),
                                                 pSimulationState->componentPools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeRenderStatePool;
        ++pPool->refCount;
        pSimulationState->componentPools.emplace_back(pPool);
    }

    // Systems
    pSimulationState->systems.emplace_back(new foeArmatureSystem);
    pSimulationState->systems.emplace_back(new foeCameraSystem);
    pSimulationState->systems.emplace_back(new PositionDescriptorPool);
    pSimulationState->systems.emplace_back(new VkAnimationPool);
}

void onDestroy(foeSimulationState *pSimulationState) {
    // Systems
    for (auto &pSystem : pSimulationState->systems) {
        searchAndDestroy<VkAnimationPool>(pSystem);
        searchAndDestroy<PositionDescriptorPool>(pSystem);
        searchAndDestroy<foeCameraSystem>(pSystem);
        searchAndDestroy<foeArmatureSystem>(pSystem);
    }

    // Components
    for (auto &pPool : pSimulationState->componentPools) {
        searchAndDestroy<foeRenderStatePool>(pPool);
        searchAndDestroy<foeCameraPool>(pPool);
        searchAndDestroy<foeArmatureStatePool>(pPool);
    }
}

struct Initialized {
    // Systems
    bool armature;
    bool camera;
    bool positionDescriptor;
    bool animation;
};

void deinitialize(Initialized const &initialized, foeSimulationStateLists const *pSimStateData) {
    // Systems
    auto *pIt = pSimStateData->pSystems;
    auto const *pEndIt = pSimStateData->pSystems + pSimStateData->systemCount;

    for (; pIt != pEndIt; ++pIt) {
        if (initialized.armature)
            searchAndDeinit<foeArmatureSystem>(*pIt);
        if (initialized.camera)
            searchAndDeinit<foeCameraSystem>(*pIt);
        if (initialized.positionDescriptor)
            searchAndDeinit<PositionDescriptorPool>(*pIt);
        if (initialized.animation)
            searchAndDeinit<VkAnimationPool>(*pIt);
    }
}

std::error_code onInitialization(foeSimulationInitInfo const *pInitInfo,
                                 foeSimulationStateLists const *pSimStateData) {
    std::error_code errC;
    Initialized initialized{};

    // Systems
    auto *pIt = pSimStateData->pSystems;
    auto const *pEndIt = pSimStateData->pSystems + pSimStateData->systemCount;

    for (; pIt != pEndIt; ++pIt) {
        if (auto *pArmatureSystem = dynamic_cast<foeArmatureSystem *>(*pIt); pArmatureSystem) {
            ++pArmatureSystem->initCount;
            if (pArmatureSystem->initialized())
                continue;

            auto *pArmaturePool = search<foeArmaturePool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

            auto *pArmatureStatePool = search<foeArmatureStatePool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            pArmatureSystem->initialize(pArmaturePool, pArmatureStatePool);
            initialized.armature = true;
        }

        if (auto *pCameraSystem = dynamic_cast<foeCameraSystem *>(*pIt); pCameraSystem) {
            ++pCameraSystem->initCount;
            if (pCameraSystem->initialized())
                continue;

            auto *pPosition3dPool = search<foePosition3dPool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            auto *pCameraPool = search<foeCameraPool>(pSimStateData->pComponentPools,
                                                      pSimStateData->pComponentPools +
                                                          pSimStateData->componentPoolCount);

            errC = pCameraSystem->initialize(pPosition3dPool, pCameraPool, pInitInfo->gfxSession);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.camera = true;
        }

        if (auto *pPositionDescriptorPool = dynamic_cast<PositionDescriptorPool *>(*pIt);
            pPositionDescriptorPool) {
            ++pPositionDescriptorPool->initCount;
            if (pPositionDescriptorPool->initialized())
                continue;

            auto *pPosition3dPool = search<foePosition3dPool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            errC = pPositionDescriptorPool->initialize(pPosition3dPool, pInitInfo->gfxSession);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.positionDescriptor = true;
        }

        if (auto *pVkAnimationPool = dynamic_cast<VkAnimationPool *>(*pIt); pVkAnimationPool) {
            ++pVkAnimationPool->initCount;
            if (pVkAnimationPool->initialized())
                continue;

            auto *pArmaturePool = search<foeArmaturePool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

            auto *pMeshPool = search<foeMeshPool>(pSimStateData->pResourcePools,
                                                  pSimStateData->pResourcePools +
                                                      pSimStateData->resourcePoolCount);

            auto *pArmatureStatePool = search<foeArmatureStatePool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            auto *pRenderStatePool = search<foeRenderStatePool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            errC = pVkAnimationPool->initialize(pArmaturePool, pMeshPool, pArmatureStatePool,
                                                pRenderStatePool, pInitInfo->gfxSession);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.animation = true;
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitialize(initialized, pSimStateData);

    return errC;
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Systems
    for (auto *pSystem : pSimulationState->systems) {
        searchAndDeinit<VkAnimationPool>(pSystem);
        searchAndDeinit<PositionDescriptorPool>(pSystem);
        searchAndDeinit<foeCameraSystem>(pSystem);
        searchAndDeinit<foeArmatureSystem>(pSystem);
    }
}

} // namespace

void foeBringupRegisterFunctionality() {
    FOE_LOG(foeBringup, Verbose,
            "foeBringupRegisterFunctionality - Starting to register functionality")

    foeRegisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    FOE_LOG(foeBringup, Verbose,
            "foeBringupRegisterFunctionality - Completed registering functionality")
}

void foeBringupDeregisterFunctionality() {
    FOE_LOG(foeBringup, Verbose,
            "foeBringupDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    FOE_LOG(foeBringup, Verbose,
            "foeBringupDeregisterFunctionality - Completed deregistering functionality")
}