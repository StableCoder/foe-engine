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

#include <foe/graphics/resource/registrar.hpp>

#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/material_pool.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/simulation/group_data.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
#include "log.hpp"

namespace {

foeResourceCreateInfoBase *importFn(void *pContext, foeResourceID resource) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);
    return pGroupData->getResourceDefinition(resource);
}

void imageLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pImage = reinterpret_cast<foeImage *>(pResource);

    auto pLocalCreateInfo = pImage->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pImage, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void materialLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pMaterial = reinterpret_cast<foeMaterial *>(pResource);

    auto pLocalCreateInfo = pMaterial->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pMaterial, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void shaderLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pShader = reinterpret_cast<foeShader *>(pResource);

    auto pLocalCreateInfo = pShader->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pShader, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void vertexDescriptorLoadFn(void *pContext,
                            void *pResource,
                            void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pVertexDescriptor = reinterpret_cast<foeVertexDescriptor *>(pResource);

    auto pLocalCreateInfo = pVertexDescriptor->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pVertexDescriptor, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void onCreate(foeSimulationState *pSimulationState) {
    // Resources
    pSimulationState->resourcePools.emplace_back(new foeImagePool{foeResourceFns{
        .pImportContext = &pSimulationState->groupData,
        .pImportFn = importFn,
        .pLoadContext = pSimulationState,
        .pLoadFn = imageLoadFn,
        .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
    }});

    pSimulationState->resourcePools.emplace_back(new foeMaterialPool{foeResourceFns{
        .pImportContext = &pSimulationState->groupData,
        .pImportFn = importFn,
        .pLoadContext = pSimulationState,
        .pLoadFn = materialLoadFn,
        .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
    }});

    pSimulationState->resourcePools.emplace_back(new foeShaderPool{foeResourceFns{
        .pImportContext = &pSimulationState->groupData,
        .pImportFn = importFn,
        .pLoadContext = pSimulationState,
        .pLoadFn = shaderLoadFn,
        .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
    }});

    pSimulationState->resourcePools.emplace_back(new foeVertexDescriptorPool{foeResourceFns{
        .pImportContext = &pSimulationState->groupData,
        .pImportFn = importFn,
        .pLoadContext = pSimulationState,
        .pLoadFn = materialLoadFn,
        .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
    }});

    // Loaders
    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = new foeImageLoader, .pGfxMaintenanceFn = [](foeResourceLoaderBase *pLoader) {
            auto *pImageLoader = reinterpret_cast<foeImageLoader *>(pLoader);
            pImageLoader->gfxMaintenance();
        }});

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = new foeMaterialLoader, .pGfxMaintenanceFn = [](foeResourceLoaderBase *pLoader) {
            auto *pMaterialLoader = reinterpret_cast<foeMaterialLoader *>(pLoader);
            pMaterialLoader->gfxMaintenance();
        }});

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = new foeShaderLoader, .pGfxMaintenanceFn = [](foeResourceLoaderBase *pLoader) {
            auto *pShaderLoader = reinterpret_cast<foeShaderLoader *>(pLoader);
            pShaderLoader->gfxMaintenance();
        }});

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = new foeVertexDescriptorLoader,
        .pGfxMaintenanceFn = [](foeResourceLoaderBase *pLoader) {
            auto *pVertexDescriptorLoader = reinterpret_cast<foeVertexDescriptorLoader *>(pLoader);
            pVertexDescriptorLoader->gfxMaintenance();
        }});
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
    // Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy<foeVertexDescriptorLoader>(it.pLoader);
        searchAndDestroy<foeShaderLoader>(it.pLoader);
        searchAndDestroy<foeMaterialLoader>(it.pLoader);
        searchAndDestroy<foeImageLoader>(it.pLoader);
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy<foeVertexDescriptorLoader>(pPool);
        searchAndDestroy<foeShaderPool>(pPool);
        searchAndDestroy<foeMaterialPool>(pPool);
        searchAndDestroy<foeImagePool>(pPool);
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
    { // Loaders
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pImageLoader = dynamic_cast<foeImageLoader *>(pIt->pLoader);
            if (pImageLoader) {
                pImageLoader->initialize(pInitInfo->gfxSession, pInitInfo->externalFileSearchFn);
            }

            auto *pMaterialLoader = dynamic_cast<foeMaterialLoader *>(pIt->pLoader);
            if (pMaterialLoader) {
                auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

                auto *pImagePool = search<foeImagePool>(pSimStateData->pResourcePools,
                                                        pSimStateData->pResourcePools +
                                                            pSimStateData->resourcePoolCount);

                pMaterialLoader->initialize(pShaderPool, pImagePool, pInitInfo->gfxSession);
            }

            auto *pShaderLoader = dynamic_cast<foeShaderLoader *>(pIt->pLoader);
            if (pShaderLoader) {
                pShaderLoader->initialize(pInitInfo->gfxSession, pInitInfo->externalFileSearchFn);
            }

            auto *pVertexDescriptorLoader = dynamic_cast<foeVertexDescriptorLoader *>(pIt->pLoader);
            if (pVertexDescriptorLoader) {
                auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

                pVertexDescriptorLoader->initialize(pShaderPool);
            }
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
    // Loaders
    for (auto const &it : pSimulationState->resourceLoaders) {
        searchAndDeinit<foeVertexDescriptorLoader>(it.pLoader);
        searchAndDeinit<foeShaderLoader>(it.pLoader);
        searchAndDeinit<foeMaterialLoader>(it.pLoader);
        searchAndDeinit<foeImageLoader>(it.pLoader);
    }
}

} // namespace

auto foeGraphicsResourceRegisterFunctionality() -> std::error_code {
    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    if (errC) {
        FOE_LOG(
            foeGraphicsResource, Error,
            "foeGraphicsResourceRegisterFunctionality - Failed registering functionality: {} - {}",
            errC.value(), errC.message())
    } else {
        FOE_LOG(foeGraphicsResource, Verbose,
                "foeGraphicsResourceRegisterFunctionality - Completed registering functionality")
    }

    return errC;
}

void foeGraphicsResourceDeregisterFunctionality() {
    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeSimulationFunctionalty{
        .onCreate = onCreate,
        .onDestroy = onDestroy,
        .onInitialization = onInitialization,
        .onDeinitialization = onDeinitialization,
    });

    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceDeregisterFunctionality - Completed deregistering functionality")
}