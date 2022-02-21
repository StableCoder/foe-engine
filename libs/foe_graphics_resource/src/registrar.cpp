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
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }

    // Loaders
    if (auto *pLoader = searchLoaders<foeImageLoader>(pSimulationState->resourceLoaders.begin(),
                                                      pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeImageLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pImageLoader = reinterpret_cast<foeImageLoader *>(pLoader);
                    pImageLoader->gfxMaintenance();
                },
        });
    }

    if (auto *pLoader = searchLoaders<foeMaterialLoader>(pSimulationState->resourceLoaders.begin(),
                                                         pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeMaterialLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pMaterialLoader = reinterpret_cast<foeMaterialLoader *>(pLoader);
                    pMaterialLoader->gfxMaintenance();
                },
        });
    }

    if (auto *pLoader = searchLoaders<foeShaderLoader>(pSimulationState->resourceLoaders.begin(),
                                                       pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeShaderLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pShaderLoader = reinterpret_cast<foeShaderLoader *>(pLoader);
                    pShaderLoader->gfxMaintenance();
                },
        });
    }

    if (auto *pLoader = searchLoaders<foeVertexDescriptorLoader>(
            pSimulationState->resourceLoaders.begin(), pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeVertexDescriptorLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pVertexDescriptorLoader =
                        reinterpret_cast<foeVertexDescriptorLoader *>(pLoader);
                    pVertexDescriptorLoader->gfxMaintenance();
                },
        });
    }

    if (auto *pLoader = searchLoaders<foeMeshLoader>(pSimulationState->resourceLoaders.begin(),
                                                     pSimulationState->resourceLoaders.end());
        pLoader) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeMeshLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pMeshLoader = reinterpret_cast<foeMeshLoader *>(pLoader);
                    pMeshLoader->gfxMaintenance();
                },
        });
    }
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
        searchAndDestroy<foeMeshPool>(pPool);
        searchAndDestroy<foeVertexDescriptorPool>(pPool);
        searchAndDestroy<foeShaderPool>(pPool);
        searchAndDestroy<foeMaterialPool>(pPool);
        searchAndDestroy<foeImagePool>(pPool);
    }
}

struct Initialized {
    bool image;
    bool material;
    bool shader;
    bool vertexDescriptor;
    bool mesh;
};

void deinitialize(Initialized const &initialized, foeSimulationStateLists const *pSimStateData) {
    // Loaders
    auto *pIt = pSimStateData->pResourceLoaders;
    auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

    for (; pIt != pEndIt; ++pIt) {
        if (initialized.image)
            searchAndDeinit<foeImageLoader>(pIt->pLoader);
        if (initialized.material)
            searchAndDeinit<foeMaterialLoader>(pIt->pLoader);
        if (initialized.shader)
            searchAndDeinit<foeShaderLoader>(pIt->pLoader);
        if (initialized.vertexDescriptor)
            searchAndDeinit<foeVertexDescriptorLoader>(pIt->pLoader);
        if (initialized.mesh)
            searchAndDeinit<foeMeshLoader>(pIt->pLoader);
    }
}

std::error_code onInitialization(foeSimulationInitInfo const *pInitInfo,
                                 foeSimulationStateLists const *pSimStateData) {
    std::error_code errC;
    Initialized initialized{};

    { // Loaders
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            if (auto *pImageLoader = dynamic_cast<foeImageLoader *>(pIt->pLoader); pImageLoader) {
                ++pImageLoader->initCount;
                if (pImageLoader->initialized())
                    continue;

                errC = pImageLoader->initialize(pInitInfo->externalFileSearchFn);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.image = true;
            }

            if (auto *pMaterialLoader = dynamic_cast<foeMaterialLoader *>(pIt->pLoader);
                pMaterialLoader) {
                ++pMaterialLoader->initCount;
                if (pMaterialLoader->initialized())
                    continue;

                auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

                auto *pImagePool = search<foeImagePool>(pSimStateData->pResourcePools,
                                                        pSimStateData->pResourcePools +
                                                            pSimStateData->resourcePoolCount);

                errC = pMaterialLoader->initialize(pShaderPool, pImagePool);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.material = true;
            }

            if (auto *pShaderLoader = dynamic_cast<foeShaderLoader *>(pIt->pLoader);
                pShaderLoader) {
                ++pShaderLoader->initCount;
                if (pShaderLoader->initialized())
                    continue;

                errC = pShaderLoader->initialize(pInitInfo->externalFileSearchFn);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.shader = true;
            }

            if (auto *pVertexDescriptorLoader =
                    dynamic_cast<foeVertexDescriptorLoader *>(pIt->pLoader);
                pVertexDescriptorLoader) {
                ++pVertexDescriptorLoader->initCount;
                if (pVertexDescriptorLoader->initialized()) {
                    continue;
                }

                auto *pShaderPool = search<foeShaderPool>(pSimStateData->pResourcePools,
                                                          pSimStateData->pResourcePools +
                                                              pSimStateData->resourcePoolCount);

                errC = pVertexDescriptorLoader->initialize(pShaderPool);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.vertexDescriptor = true;
            }

            if (auto *pMeshLoader = dynamic_cast<foeMeshLoader *>(pIt->pLoader); pMeshLoader) {
                ++pMeshLoader->initCount;
                if (pMeshLoader->initialized())
                    continue;

                errC = pMeshLoader->initialize(pInitInfo->externalFileSearchFn);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.mesh = true;
            }
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitialize(initialized, pSimStateData);

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

void deinitializeGraphics(Initialized const &initialized,
                          foeSimulationStateLists const *pSimStateData) {
    // Loaders
    auto *pIt = pSimStateData->pResourceLoaders;
    auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

    for (; pIt != pEndIt; ++pIt) {
        if (initialized.image)
            searchAndDeinitGraphics<foeImageLoader>(pIt->pLoader);
        if (initialized.material)
            searchAndDeinitGraphics<foeMaterialLoader>(pIt->pLoader);
        if (initialized.shader)
            searchAndDeinitGraphics<foeShaderLoader>(pIt->pLoader);
        if (initialized.mesh)
            searchAndDeinitGraphics<foeMeshLoader>(pIt->pLoader);
    }
}

std::error_code onGfxInitialization(foeSimulationStateLists const *pSimStateData,
                                    foeGfxSession gfxSession) {
    std::error_code errC;
    Initialized initialized{};

    { // Loaders
        auto *pIt = pSimStateData->pResourceLoaders;
        auto const *pEndIt = pSimStateData->pResourceLoaders + pSimStateData->resourceLoaderCount;

        for (; pIt != pEndIt; ++pIt) {
            if (auto *pImageLoader = dynamic_cast<foeImageLoader *>(pIt->pLoader); pImageLoader) {
                if (pImageLoader->initializedGraphics())
                    continue;

                errC = pImageLoader->initializeGraphics(gfxSession);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.image = true;
            }

            if (auto *pMaterialLoader = dynamic_cast<foeMaterialLoader *>(pIt->pLoader);
                pMaterialLoader) {
                if (pMaterialLoader->initializedGraphics())
                    continue;

                errC = pMaterialLoader->initializeGraphics(gfxSession);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.material = true;
            }

            if (auto *pShaderLoader = dynamic_cast<foeShaderLoader *>(pIt->pLoader);
                pShaderLoader) {
                if (pShaderLoader->initializedGraphics())
                    continue;

                errC = pShaderLoader->initializeGraphics(gfxSession);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.shader = true;
            }

            if (auto *pMeshLoader = dynamic_cast<foeMeshLoader *>(pIt->pLoader); pMeshLoader) {
                if (pMeshLoader->initializedGraphics())
                    continue;

                errC = pMeshLoader->initializeGraphics(gfxSession);
                if (errC)
                    goto INITIALIZATION_FAILED;
                initialized.mesh = true;
            }
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitializeGraphics(initialized, pSimStateData);

    return errC;
}

void onGfxDeinitialization(foeSimulationState const *pSimulationState) {
    // Loaders
    for (auto const &it : pSimulationState->resourceLoaders) {
        searchAndDeinitGraphics<foeMeshLoader>(it.pLoader);
        searchAndDeinitGraphics<foeShaderLoader>(it.pLoader);
        searchAndDeinitGraphics<foeMaterialLoader>(it.pLoader);
        searchAndDeinitGraphics<foeImageLoader>(it.pLoader);
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
        .onGfxInitialization = onGfxInitialization,
        .onGfxDeinitialization = onGfxDeinitialization,
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
        .onGfxInitialization = onGfxInitialization,
        .onGfxDeinitialization = onGfxDeinitialization,
    });

    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceDeregisterFunctionality - Completed deregistering functionality")
}