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

#include <foe/position/component/3d_pool.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

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
    pSimulationState->componentPools.emplace_back(new foeArmatureStatePool);
    pSimulationState->componentPools.emplace_back(new foeCameraPool);
    pSimulationState->componentPools.emplace_back(new foeRenderStatePool);

    // Systems
    pSimulationState->systems.emplace_back(new foeArmatureSystem);
    pSimulationState->systems.emplace_back(new foeCameraSystem);
    pSimulationState->systems.emplace_back(new PositionDescriptorPool);
    pSimulationState->systems.emplace_back(new VkAnimationPool);
}

template <typename DestroyType, typename InType>
void searchAndDestroy(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr) {
        delete dynPtr;
        ptr = nullptr;
    }
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

template <typename SearchType, typename InputIt>
SearchType *search(InputIt start, InputIt end) noexcept {
    for (; start != end; ++start) {
        auto *dynPtr = dynamic_cast<SearchType *>(*start);
        if (dynPtr)
            return dynPtr;
    }

    return nullptr;
}

void onInitialization(foeSimulationInitInfo const *pInitInfo,
                      foeSimulationStateLists const *pSimStateData) {
    // Systems
    auto *pIt = pSimStateData->pSystems;
    auto const *pEndIt = pSimStateData->pSystems + pSimStateData->systemCount;

    for (; pIt != pEndIt; ++pIt) {
        auto *pArmatureSystem = dynamic_cast<foeArmatureSystem *>(*pIt);
        if (pArmatureSystem) {
            auto *pArmaturePool = search<foeArmaturePool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

            auto *pArmatureStatePool = search<foeArmatureStatePool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            pArmatureSystem->initialize(pArmaturePool, pArmatureStatePool);
        }

        auto *pCameraSystem = dynamic_cast<foeCameraSystem *>(*pIt);
        if (pCameraSystem) {
            auto *pPosition3dPool = search<foePosition3dPool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            auto *pCameraPool = search<foeCameraPool>(pSimStateData->pComponentPools,
                                                      pSimStateData->pComponentPools +
                                                          pSimStateData->componentPoolCount);

            pCameraSystem->initialize(pPosition3dPool, pCameraPool, pInitInfo->gfxSession);
        }

        auto *pPositionDescriptorPool = dynamic_cast<PositionDescriptorPool *>(*pIt);
        if (pPositionDescriptorPool) {
            auto *pPosition3dPool = search<foePosition3dPool>(
                pSimStateData->pComponentPools,
                pSimStateData->pComponentPools + pSimStateData->componentPoolCount);

            pPositionDescriptorPool->initialize(pPosition3dPool, pInitInfo->gfxSession);
        }

        auto *pVkAnimationPool = dynamic_cast<VkAnimationPool *>(*pIt);
        if (pVkAnimationPool) {
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

            pVkAnimationPool->initialize(pArmaturePool, pMeshPool, pArmatureStatePool,
                                         pRenderStatePool, pInitInfo->gfxSession);
        }
    }
}

template <typename DestroyType, typename InType>
void searchAndDeinit(InType &ptr) noexcept {
    auto *dynPtr = dynamic_cast<DestroyType *>(ptr);
    if (dynPtr) {
        dynPtr->deinitialize();
    }
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