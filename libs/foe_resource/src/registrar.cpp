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

#include <foe/resource/registrar.hpp>

#include <foe/resource/armature.hpp>
#include <foe/resource/armature_loader.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/type_defs.h>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/registration_fn_templates.hpp>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
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
    bool poolFound = false;

    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL) {
            ++pPool->refCount;
            poolFound = true;
        }
    }

    if (!poolFound) {
        auto *pPool = new foeArmaturePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = armatureLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    // Resource Loaders
    if (auto *pLoader = searchLoaders<foeArmatureLoader>(pSimulationState->resourceLoaders.begin(),
                                                         pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeArmatureLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pArmatureLoader = reinterpret_cast<foeArmatureLoader *>(pLoader);
                    pArmatureLoader->maintenance();
                },
        });
    }
}

void onDestroy(foeSimulationState *pSimulationState) {
    // Resource Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy<foeArmatureLoader>(it.pLoader);
    }

    // Resource Pools
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL) {
            auto *pTemp = (foeArmaturePool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        }
    }
}

std::error_code onInitialize(foeSimulationInitInfo const *pInitInfo,
                             foeSimulationStateLists const *pSimStateData) {
    std::error_code errC;

    // Resource Loaders
    auto *pIt = pSimStateData->pResourceLoaders;
    auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

    for (; pIt != pEndIt; ++pIt) {
        if (auto *pArmatureLoader = dynamic_cast<foeArmatureLoader *>(pIt->pLoader);
            pArmatureLoader) {
            ++pArmatureLoader->initCount;
            if (pArmatureLoader->initialized())
                continue;

            errC = pArmatureLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            searchAndDeinit<foeArmatureLoader>(pIt->pLoader);
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

int foeResourceFunctionalityID() { return FOE_RESOURCE_FUNCTIONALITY_ID; }

auto foeResourceRegisterFunctionality() -> std::error_code {
    FOE_LOG(foeResource, Verbose,
            "foeResourceRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foeResourceFunctionalityID(),
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialize,
        .onDeinitialization = onDeinitialize,
    });

    if (errC) {
        FOE_LOG(foeResource, Error,
                "foeResourceRegisterFunctionality - Failed registering functionality: {} - {}",
                errC.value(), errC.message())
    } else {
        FOE_LOG(foeResource, Verbose,
                "foeResourceRegisterFunctionality - Completed registering functionality")
    }

    return errC;
}

void foeResourceDeregisterFunctionality() {
    FOE_LOG(foeResource, Verbose,
            "foeResourceDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .id = foeResourceFunctionalityID(),
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialize,
        .onDeinitialization = onDeinitialize,
    });

    FOE_LOG(foeResource, Verbose,
            "foeResourceDeregisterFunctionality - Completed deregistering functionality")
}