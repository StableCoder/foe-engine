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
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
#include "log.hpp"

namespace {

struct TypeSelection {
    // Resources
    bool armatureResources;
    // Loaders
    bool armatureLoader;
};

foeResourceCreateInfoBase *importFn(void *pContext, foeResourceID resource) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);
    return pGroupData->getResourceDefinition(resource);
}

void armatureLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pArmature = reinterpret_cast<foeArmature *>(pResource);

    auto pLocalCreateInfo = pArmature->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pArmature, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

bool destroySelected(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    std::error_code errC;
    size_t count;
    bool cleanRun = true;

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        errC = foeSimulationDecrementRefCount(pSimulationState,
                                              FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (errC) {
            // Trying to destroy something that doesn't exist? Not optimal
            FOE_LOG(foeResource, Warning,
                    "Attempted to decrement/destroy ArmatureLoader that doesn't exist - {}",
                    errC.message());
            cleanRun = false;
        } else if (count == 0) {
            foeArmatureLoader *pLoader;
            errC = foeSimulationReleaseResourceLoader(
                pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER, (void **)&pLoader);
            if (errC) {
                FOE_LOG(foeResource, Warning, "Could not release ArmatureLoader to destroy - {}",
                        errC.message());
                cleanRun = false;
            } else {
                delete pLoader;
            }
        }
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if ((pSelection == nullptr || pSelection->armatureResources) &&
            pPool->sType == FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL) {
            auto *pTemp = (foeArmaturePool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        }
    }

    return cleanRun;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    std::error_code errC;
    TypeSelection selected = {};

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_POOL) {
            ++pPool->refCount;
            selected.armatureResources = true;
        }
    }

    if (!selected.armatureResources) {
        auto *pPool = new foeArmaturePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = armatureLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
        selected.armatureResources = true;
    }

    // Loaders
    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER,
            .pLoader = new foeArmatureLoader,
            .pCanProcessCreateInfoFn = foeArmatureLoader::canProcessCreateInfo,
            .pLoadFn = foeArmatureLoader::load,
            .pMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeArmatureLoader *>(pLoader)->maintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeArmatureLoader *)loaderCI.pLoader;
            FOE_LOG(foeResource, Error,
                    "onCreate - Failed to create foeArmatureLoader on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER, nullptr);
    }
    selected.armatureLoader = true;

CREATE_FAILED:
    if (errC)
        destroySelected(pSimulationState, &selected);

    return errC;
}

bool destroy(foeSimulationState *pSimulationState) {
    return destroySelected(pSimulationState, nullptr);
}

bool deinitializeSelected(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    size_t count;
    bool cleanRun = true;

    // Loaders
    if (pSelection == nullptr || pSelection->armatureLoader) {
        auto errC = foeSimulationDecrementInitCount(
            pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER, &count);
        if (errC) {
            FOE_LOG(foeResource, Warning,
                    "Failed to decrement foeArmatureLoader initialization count on Simulation {} "
                    "with error {}",
                    (void *)pSimulationState, errC.message());
            cleanRun = false;
        } else if (count == 0) {
            auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
                pSimulationState, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER);
            pLoader->deinitialize();
        }
    }

    return cleanRun;
}

std::error_code initialize(foeSimulationState *pSimulation,
                           foeSimulationInitInfo const *pInitInfo) {
    TypeSelection selection = {};
    std::error_code errC;
    size_t count;

    // Loaders
    errC = foeSimulationIncrementInitCount(pSimulation, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER,
                                           &count);
    if (errC) {
        FOE_LOG(foeResource, Error,
                "Failed to increment foeArmatureLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.armatureLoader = true;
        auto *pLoader = (foeArmatureLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_RESOURCE_STRUCTURE_TYPE_ARMATURE_LOADER);
        errC = pLoader->initialize(pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeResource, Error,
                    "Failed to initialize foeArmatureLoader on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        deinitializeSelected(pSimulation, &selection);
    }

    return errC;
}

bool deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelected(pSimulationState, nullptr);
}

} // namespace

int foeResourceFunctionalityID() { return FOE_RESOURCE_FUNCTIONALITY_ID; }

auto foeResourceRegisterFunctionality() -> std::error_code {
    FOE_LOG(foeResource, Verbose,
            "foeResourceRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foeResourceFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
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

    foeDeregisterFunctionality(foeResourceFunctionalityID());

    FOE_LOG(foeResource, Verbose,
            "foeResourceDeregisterFunctionality - Completed deregistering functionality")
}