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
#include <foe/graphics/resource/mesh_loader.hpp>
#include <foe/graphics/resource/mesh_pool.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/simulation/group_data.hpp>
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

void meshLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pMesh = reinterpret_cast<foeMesh *>(pResource);

    auto pLocalCreateInfo = pMesh->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pLoader->canProcessCreateInfo(pLocalCreateInfo.get())) {
            return it.pLoader->load(pMesh, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void onCreate(foeSimulationState *pSimulationState) {
    // Resources
    if (auto *pPool = search<foeImagePool>(pSimulationState->resourcePools.begin(),
                                           pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeImagePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = imageLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    if (auto *pPool = search<foeMaterialPool>(pSimulationState->resourcePools.begin(),
                                              pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeMaterialPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = materialLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    if (auto *pPool = search<foeShaderPool>(pSimulationState->resourcePools.begin(),
                                            pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeShaderPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = shaderLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    if (auto *pPool = search<foeVertexDescriptorPool>(pSimulationState->resourcePools.begin(),
                                                      pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeVertexDescriptorPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = vertexDescriptorLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    if (auto *pPool = search<foeMeshPool>(pSimulationState->resourcePools.begin(),
                                          pSimulationState->resourcePools.end());
        pPool) {
        ++pPool->refCount;
    } else {
        pPool = new foeMeshPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = meshLoadFn,
            .asyncTaskFn = pSimulationState->createInfo.asyncTaskFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

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

    pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
        .pLoader = new foeMeshLoader, .pGfxMaintenanceFn = [](foeResourceLoaderBase *pLoader) {
            auto *pMeshLoader = reinterpret_cast<foeMeshLoader *>(pLoader);
            pMeshLoader->gfxMaintenance();
        }});
}

void onDestroy(foeSimulationState *pSimulationState) {
    // Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        searchAndDestroy<foeMeshLoader>(it.pLoader);
        searchAndDestroy<foeVertexDescriptorLoader>(it.pLoader);
        searchAndDestroy<foeShaderLoader>(it.pLoader);
        searchAndDestroy<foeMaterialLoader>(it.pLoader);
        searchAndDestroy<foeImageLoader>(it.pLoader);
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        searchAndDestroy2<foeMeshPool>(pPool);
        searchAndDestroy2<foeVertexDescriptorPool>(pPool);
        searchAndDestroy2<foeShaderPool>(pPool);
        searchAndDestroy2<foeMaterialPool>(pPool);
        searchAndDestroy2<foeImagePool>(pPool);
    }
}

std::error_code onInitialization(foeSimulationInitInfo const *pInitInfo,
                      foeSimulationStateLists const *pSimStateData) {
    std::error_code errC;
    { // Loaders
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            auto *pImageLoader = dynamic_cast<foeImageLoader *>(pIt->pLoader);
            if (pImageLoader) {
                errC = pImageLoader->initialize(pInitInfo->gfxSession,
                                                pInitInfo->externalFileSearchFn);
                if (errC)
                    goto INITIALIZATION_FAILED;
            }

            auto *pMaterialLoader = dynamic_cast<foeMaterialLoader *>(pIt->pLoader);
            if (pMaterialLoader) {
                auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

                auto *pImagePool = search<foeImagePool>(pSimStateData->pResourcePools,
                                                        pSimStateData->pResourcePools +
                                                            pSimStateData->resourcePoolCount);

                errC = pMaterialLoader->initialize(pShaderPool, pImagePool, pInitInfo->gfxSession);
                if (errC)
                    goto INITIALIZATION_FAILED;
            }

            auto *pShaderLoader = dynamic_cast<foeShaderLoader *>(pIt->pLoader);
            if (pShaderLoader) {
                errC = pShaderLoader->initialize(pInitInfo->gfxSession,
                                                 pInitInfo->externalFileSearchFn);
                if (errC)
                    goto INITIALIZATION_FAILED;
            }

            auto *pVertexDescriptorLoader = dynamic_cast<foeVertexDescriptorLoader *>(pIt->pLoader);
            if (pVertexDescriptorLoader) {
                auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

                errC = pVertexDescriptorLoader->initialize(pShaderPool);
                if (errC)
                    goto INITIALIZATION_FAILED;
            }

            auto *pMeshLoader = dynamic_cast<foeMeshLoader *>(pIt->pLoader);
            if (pMeshLoader) {
                errC =
                pMeshLoader->initialize(pInitInfo->gfxSession, pInitInfo->externalFileSearchFn);
                if (errC)
                    goto INITIALIZATION_FAILED;
            }
        }
    }

INITIALIZATION_FAILED:
    return errC;
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Loaders
    for (auto const &it : pSimulationState->resourceLoaders) {
        searchAndDeinit<foeMeshLoader>(it.pLoader);
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