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

#include <foe/resource/armature_loader.hpp>
#include <foe/resource/armature_pool.hpp>
#include <foe/resource/mesh_loader.hpp>
#include <foe/resource/mesh_pool.hpp>
#include <foe/resource/shader_loader.hpp>
#include <foe/resource/shader_pool.hpp>
#include <foe/resource/vertex_descriptor_loader.hpp>
#include <foe/resource/vertex_descriptor_pool.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "log.hpp"

namespace {

void onCreate(foeSimulationState *pSimulationState) {
    auto *pArmatureLoader = new foeArmatureLoader;
    auto *pShaderLoader = new foeShaderLoader;
    auto *pMeshLoader = new foeMeshLoader;
    auto *pVertexDescriptorLoader = new foeVertexDescriptorLoader;

    // Resource Pools
    pSimulationState->resourcePools.emplace_back(new foeArmaturePool{pArmatureLoader});
    pSimulationState->resourcePools.emplace_back(new foeMeshPool{pMeshLoader});
    pSimulationState->resourcePools.emplace_back(new foeShaderPool{pShaderLoader});
    pSimulationState->resourcePools.emplace_back(
        new foeVertexDescriptorPool{pVertexDescriptorLoader});

    // Resource Loaders
    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = pArmatureLoader,
    });

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = pMeshLoader,
    });

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = pShaderLoader,
    });

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = pVertexDescriptorLoader,
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
        searchAndDestroy<foeVertexDescriptorLoader>(it.pLoader);
        searchAndDestroy<foeShaderLoader>(it.pLoader);
        searchAndDestroy<foeMeshLoader>(it.pLoader);
        searchAndDestroy<foeArmatureLoader>(it.pLoader);
    }

    // Resource Pools
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy<foeVertexDescriptorPool>(pPool);
        searchAndDestroy<foeShaderPool>(pPool);
        searchAndDestroy<foeMeshPool>(pPool);
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
        auto *pArmatureLoader = dynamic_cast<foeArmatureLoader *>(pIt->pLoader);
        if (pArmatureLoader) {
            pArmatureLoader->initialize(pInitInfo->resourceDefinitionImportFn,
                                        pInitInfo->externalFileSearchFn, pInitInfo->asyncJobFn);
        }

        auto *pMeshLoader = dynamic_cast<foeMeshLoader *>(pIt->pLoader);
        if (pMeshLoader) {
            pMeshLoader->initialize(pInitInfo->gfxSession, pInitInfo->resourceDefinitionImportFn,
                                    pInitInfo->externalFileSearchFn, pInitInfo->asyncJobFn);
        }

        auto *pShaderLoader = dynamic_cast<foeShaderLoader *>(pIt->pLoader);
        if (pShaderLoader) {
            pShaderLoader->initialize(pInitInfo->gfxSession, pInitInfo->resourceDefinitionImportFn,
                                      pInitInfo->externalFileSearchFn, pInitInfo->asyncJobFn);
        }

        auto *pVertexDescriptorLoader = dynamic_cast<foeVertexDescriptorLoader *>(pIt->pLoader);
        if (pVertexDescriptorLoader) {
            auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                      pSimStateData->pResourcePools +
                                                          pSimStateData->resourcePoolCount);

            pVertexDescriptorLoader->initialize(pShaderPool, pInitInfo->resourceDefinitionImportFn,
                                                pInitInfo->asyncJobFn);
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
        searchAndDeinit<foeVertexDescriptorLoader>(it.pLoader);
        searchAndDeinit<foeShaderLoader>(it.pLoader);
        searchAndDeinit<foeMeshLoader>(it.pLoader);
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