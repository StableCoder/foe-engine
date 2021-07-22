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
#include <foe/simulation/registration_fn_templates.hpp>
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
    if (auto *pPool = search<foeArmaturePool>(pSimulationState->resourcePools.begin(),
                                              pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeArmaturePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = armatureLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

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

void onDestroy(foeSimulationState *pSimulationState) {
    // Resource Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy<foeArmatureLoader>(it.pLoader);
    }

    // Resource Pools
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy2<foeArmaturePool>(pPool);
    }
}

std::error_code onInitialize(foeSimulationInitInfo const *pInitInfo,
                             foeSimulationStateLists const *pSimStateData) {
    std::error_code errC;

    // Resource Loaders
    auto *pIt = pSimStateData->pResourceLoaders;
    auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

    for (; pIt != pEndIt; ++pIt) {
        if (auto *pArmatureLoader = dynamic_cast<foeArmatureLoader *>(pIt->pLoader)) {
            errC = pArmatureLoader->initialize(pInitInfo->externalFileSearchFn);
        }
    }

    return errC;
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