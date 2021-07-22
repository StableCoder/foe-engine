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

#include <foe/resource/registrar.hpp>

#include <foe/resource/armature.hpp>
#include <foe/resource/armature_loader.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/error_code.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "log.hpp"

namespace {

foeResourceCreateInfoBase *importFn(void *pContext, foeResourceID resource) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);
    return pGroupData->getResourceDefinition(resource);
}

void armatureLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pArmature = reinterpret_cast<foeArmature *>(pResource);

    auto pLocalCreateInfo = pArmature->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pArmature, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void onCreate(foeSimulationState *pSimulationState) {
    // Resource Pools
    pSimulationState->resourcePools.emplace_back(new foeArmaturePool{foeResourceFns{
        .pImportContext = &pSimulationState->groupData,
        .pImportFn = importFn,
        .pLoadContext = pSimulationState,
        .pLoadFn = armatureLoadFn,
        .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
    }});

    // Resource Loaders
    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = new foeArmatureLoader,
        .pMaintenanceFn =
            [](foeResourceLoaderBase *pLoader) {
                auto *pArmatureLoader = reinterpret_cast<foeArmatureLoader *>(pLoader);
                pArmatureLoader->maintenance();
            },
    });
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
    // Resource Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy<foeArmatureLoader>(it.pLoader);
    }

    // Resource Pools
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy<foeArmaturePool>(pPool);
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

void onInitialize(foeSimulationInitInfo const *pInitInfo,
                  foeSimulationStateLists const *pSimStateData) {
    // Resource Loaders
    auto *pIt = pSimStateData->pResourceLoaders;
    auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

    for (; pIt != pEndIt; ++pIt) {
        if (auto *pArmatureLoader = dynamic_cast<foeArmatureLoader *>(pIt->pLoader)) {
            pArmatureLoader->initialize(pInitInfo->externalFileSearchFn);
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

void onDeinitialize(foeSimulationState const *pSimulationState) {
    // Loaders
    for (auto const &it : pSimulationState->resourceLoaders) {
        searchAndDeinit<foeArmatureLoader>(it.pLoader);
    }
}

} // namespace

void foeResourceRegisterFunctionality() {
    FOE_LOG(foeResource, Verbose,
            "foeResourceRegisterFunctionality - Starting to register functionality")

    foeRegisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialize,
        .onDeinitialization = onDeinitialize,
    });

    FOE_LOG(foeResource, Verbose,
            "foeResourceRegisterFunctionality - Completed registering functionality")
}

void foeResourceDeregisterFunctionality() {
    FOE_LOG(foeResource, Verbose,
            "foeResourceDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialize,
        .onDeinitialization = onDeinitialize,
    });

    FOE_LOG(foeResource, Verbose,
            "foeResourceDeregisterFunctionality - Completed deregistering functionality")
}