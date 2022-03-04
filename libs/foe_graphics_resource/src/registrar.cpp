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
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/graphics/resource/vertex_descriptor_pool.hpp>
#include <foe/simulation/group_data.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
#include "log.hpp"

namespace {

struct TypeSelection {
    // Resources
    bool imageResources;
    bool materialResources;
    bool shaderResources;
    bool vertexDescriptorResources;
    bool meshResources;
    // Loaders
    bool imageLoader;
    bool materialLoader;
    bool shaderLoader;
    bool vertexDescriptorLoader;
    bool meshLoader;
};

foeResourceCreateInfoBase *importFn(void *pContext, foeResourceID resource) {
    auto *pGroupData = reinterpret_cast<foeGroupData *>(pContext);
    return pGroupData->getResourceDefinition(resource);
}

void imageLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pImage = reinterpret_cast<foeImage *>(pResource);

    auto pLocalCreateInfo = pImage->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pImage, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void materialLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pMaterial = reinterpret_cast<foeMaterial *>(pResource);

    auto pLocalCreateInfo = pMaterial->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pMaterial, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void shaderLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pShader = reinterpret_cast<foeShader *>(pResource);

    auto pLocalCreateInfo = pShader->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pShader, pLocalCreateInfo, pPostLoadFn);
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
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pVertexDescriptor, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

void meshLoadFn(void *pContext, void *pResource, void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pSimulationState = reinterpret_cast<foeSimulationState *>(pContext);
    auto *pMesh = reinterpret_cast<foeMesh *>(pResource);

    auto pLocalCreateInfo = pMesh->pCreateInfo;

    for (auto const &it : pSimulationState->resourceLoaders) {
        if (it.pCanProcessCreateInfoFn(pLocalCreateInfo.get())) {
            return it.pLoadFn(it.pLoader, pMesh, pLocalCreateInfo, pPostLoadFn);
        }
    }

    pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_FAILED_TO_FIND_COMPATIBLE_LOADER);
}

#define DESTROY_LOADER(X, Y, Z)                                                                    \
    if ((pSelection == nullptr || pSelection->Z) && it.pLoader->sType == Y &&                      \
        --it.pLoader->refCount == 0) {                                                             \
        delete (X *)it.pLoader;                                                                    \
        it.pLoader = nullptr;                                                                      \
        continue;                                                                                  \
    }

bool destroySelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    // Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        if (it.pLoader == nullptr)
            continue;

        DESTROY_LOADER(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, meshLoader)
        DESTROY_LOADER(foeVertexDescriptorLoader,
                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
                       vertexDescriptorLoader)
        DESTROY_LOADER(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                       shaderLoader)
        DESTROY_LOADER(foeMaterialLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                       materialLoader)
        DESTROY_LOADER(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                       imageLoader)
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if ((pSelection == nullptr || pSelection->meshResources) &&
            pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL) {
            auto *pTemp = (foeMeshPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if ((pSelection == nullptr || pSelection->vertexDescriptorResources) &&
                   pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL) {
            auto *pTemp = (foeVertexDescriptorPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if ((pSelection == nullptr || pSelection->shaderResources) &&
                   pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL) {
            auto *pTemp = (foeShaderPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if ((pSelection == nullptr || pSelection->materialResources) &&
                   pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL) {
            auto *pTemp = (foeMaterialPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if ((pSelection == nullptr || pSelection->imageResources) &&
                   pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL) {
            auto *pTemp = (foeImagePool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        }
    }

    return true;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    TypeSelection selection = {};

    // Resources
    // Find any matching pool types that have been created already, and if so, increment the
    // reference count
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL) {
            ++pPool->refCount;
            selection.imageResources = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL) {
            ++pPool->refCount;
            selection.materialResources = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL) {
            ++pPool->refCount;
            selection.shaderResources = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL) {
            ++pPool->refCount;
            selection.vertexDescriptorResources = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL) {
            ++pPool->refCount;
            selection.meshResources = true;
        }
    }

    // For any resource pools not yet created, do so now
    if (!selection.imageResources) {
        auto *pPool = new foeImagePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = imageLoadFn,
        }};
        pSimulationState->resourcePools.emplace_back(pPool);
        ++pPool->refCount;
        selection.imageResources = true;
    }
    if (!selection.materialResources) {
        auto *pPool = new foeMaterialPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = materialLoadFn,
        }};
        pSimulationState->resourcePools.emplace_back(pPool);
        ++pPool->refCount;
        selection.materialResources = true;
    }
    if (!selection.shaderResources) {
        auto *pPool = new foeShaderPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = shaderLoadFn,
        }};
        pSimulationState->resourcePools.emplace_back(pPool);
        ++pPool->refCount;
        selection.shaderResources = true;
    }
    if (!selection.vertexDescriptorResources) {
        auto *pPool = new foeVertexDescriptorPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = vertexDescriptorLoadFn,
        }};
        pSimulationState->resourcePools.emplace_back(pPool);
        ++pPool->refCount = true;
        selection.vertexDescriptorResources = true;
    }
    if (!selection.meshResources) {
        auto *pPool = new foeMeshPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = meshLoadFn,
        }};
        pSimulationState->resourcePools.emplace_back(pPool);
        ++pPool->refCount;
        selection.meshResources = true;
    }

    // Loaders
    if (auto *pLoader = (foeImageLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);
        pLoader != nullptr) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeImageLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pCanProcessCreateInfoFn = foeImageLoader::canProcessCreateInfo,
            .pLoadFn = foeImageLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pImageLoader = reinterpret_cast<foeImageLoader *>(pLoader);
                    pImageLoader->gfxMaintenance();
                },
        });
    }
    selection.imageLoader = true;

    if (auto *pLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);
        pLoader != nullptr) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeMaterialLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pCanProcessCreateInfoFn = foeMaterialLoader::canProcessCreateInfo,
            .pLoadFn = foeMaterialLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pMaterialLoader = reinterpret_cast<foeMaterialLoader *>(pLoader);
                    pMaterialLoader->gfxMaintenance();
                },
        });
    }
    selection.materialLoader = true;

    if (auto *pLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);
        pLoader != nullptr) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeShaderLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pCanProcessCreateInfoFn = foeShaderLoader::canProcessCreateInfo,
            .pLoadFn = foeShaderLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pShaderLoader = reinterpret_cast<foeShaderLoader *>(pLoader);
                    pShaderLoader->gfxMaintenance();
                },
        });
    }
    selection.shaderLoader = true;

    if (auto *pLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);
        pLoader != nullptr) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeVertexDescriptorLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pCanProcessCreateInfoFn = foeVertexDescriptorLoader::canProcessCreateInfo,
            .pLoadFn = foeVertexDescriptorLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pVertexDescriptorLoader =
                        reinterpret_cast<foeVertexDescriptorLoader *>(pLoader);
                    pVertexDescriptorLoader->gfxMaintenance();
                },
        });
    }
    selection.vertexDescriptorLoader = true;

    if (auto *pLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);
        pLoader != nullptr) {
        ++pLoader->refCount;
    } else {
        pLoader = new foeMeshLoader;
        ++pLoader->refCount;
        pSimulationState->resourceLoaders.emplace_back(foeSimulationLoaderData{
            .pLoader = pLoader,
            .pCanProcessCreateInfoFn = foeMeshLoader::canProcessCreateInfo,
            .pLoadFn = foeMeshLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pMeshLoader = reinterpret_cast<foeMeshLoader *>(pLoader);
                    pMeshLoader->gfxMaintenance();
                },
        });
    }
    selection.meshLoader = true;

    return {};
}

bool destroy(foeSimulationState *pSimulationState) {
    return destroySelection(pSimulationState, nullptr);
}

#define DEINITIALIZE_LOADER(X, Y)                                                                  \
    if (auto *pLoader = (X *)foeSimulationGetResourceLoader(pSimulationState, Y);                  \
        pLoader != nullptr && --pLoader->initCount == 0) {                                         \
        pLoader->deinitialize();                                                                   \
    }

bool deinitializeSelection(foeSimulationState const *pSimulationState,
                           TypeSelection const *pSelection) {
    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader)
        DEINITIALIZE_LOADER(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);

    if (pSelection == nullptr || pSelection->materialLoader)
        DEINITIALIZE_LOADER(foeMaterialLoader,
                            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);

    if (pSelection == nullptr || pSelection->shaderLoader)
        DEINITIALIZE_LOADER(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader)
        DEINITIALIZE_LOADER(foeVertexDescriptorLoader,
                            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);

    if (pSelection == nullptr || pSelection->meshLoader)
        DEINITIALIZE_LOADER(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);

    return true;
}

std::error_code initialize(foeSimulationState *pSimulationState,
                           foeSimulationInitInfo const *pInitInfo) {
    std::error_code errC;
    TypeSelection selection = {};

    // Loaders
    if (auto *pImageLoader = (foeImageLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);
        pImageLoader) {
        if (!pImageLoader->initialized()) {
            errC = pImageLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pImageLoader->initCount;
        selection.imageLoader = true;
    }

    if (auto *pMaterialLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);
        pMaterialLoader) {
        if (!pMaterialLoader->initialized()) {
            auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);
            auto *pImagePool = (foeImagePool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

            errC = pMaterialLoader->initialize(pShaderPool, pImagePool);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pMaterialLoader->initCount;
        selection.materialLoader = true;
    }

    if (auto *pShaderLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);
        pShaderLoader) {
        if (!pShaderLoader->initialized()) {
            errC = pShaderLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pShaderLoader->initCount;
        selection.shaderLoader = true;
    }

    if (auto *pVertexDescriptorLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);
        pVertexDescriptorLoader) {
        if (!pVertexDescriptorLoader->initialized()) {
            auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

            errC = pVertexDescriptorLoader->initialize(pShaderPool);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pVertexDescriptorLoader->initCount;
        selection.vertexDescriptorLoader = true;
    }

    if (auto *pMeshLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);
        pMeshLoader) {
        if (!pMeshLoader->initialized()) {
            errC = pMeshLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
        }
        ++pMeshLoader->initCount;
        selection.meshLoader = true;
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitializeSelection(pSimulationState, &selection);

    return errC;
}

bool deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelection(pSimulationState, nullptr);
}

#define DEINITIALIZE_LOADER_GRAPHICS(X, Y)                                                         \
    if (auto *pLoader = (X *)foeSimulationGetResourceLoader(pSimulationState, Y);                  \
        pLoader != nullptr && --pLoader->gfxInitCount == 0) {                                      \
        pLoader->deinitializeGraphics();                                                           \
    }

bool deinitializeGraphicsSelection(foeSimulationState const *pSimulationState,
                                   TypeSelection const *pSelection) {
    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader)
        DEINITIALIZE_LOADER_GRAPHICS(foeImageLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER)
    if (pSelection == nullptr || pSelection->materialLoader)
        DEINITIALIZE_LOADER_GRAPHICS(foeMaterialLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER)
    if (pSelection == nullptr || pSelection->shaderLoader)
        DEINITIALIZE_LOADER_GRAPHICS(foeShaderLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER)
    if (pSelection == nullptr || pSelection->meshLoader)
        DEINITIALIZE_LOADER_GRAPHICS(foeMeshLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER)

    return {};
}

#define INITIALIZE_LOADER_GRAPHICS(X, Y, Z)                                                        \
    if (auto *pLoader = (X *)foeSimulationGetResourceLoader(pSimulationState, Y);                  \
        pLoader != nullptr) {                                                                      \
        if (!pLoader->initializedGraphics()) {                                                     \
            errC = pLoader->initializeGraphics(gfxSession);                                        \
            if (errC)                                                                              \
                goto INITIALIZATION_FAILED;                                                        \
            ++pLoader->gfxInitCount;                                                               \
            selection.Z = true;                                                                    \
        }                                                                                          \
    }

std::error_code initializeGraphics(foeSimulationState *pSimulationState, foeGfxSession gfxSession) {
    std::error_code errC;
    TypeSelection selection = {};

    // Loaders
    INITIALIZE_LOADER_GRAPHICS(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                               imageLoader);

    INITIALIZE_LOADER_GRAPHICS(
        foeMaterialLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, materialLoader);

    INITIALIZE_LOADER_GRAPHICS(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                               shaderLoader);

    INITIALIZE_LOADER_GRAPHICS(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
                               meshLoader);

INITIALIZATION_FAILED:
    if (errC)
        deinitializeGraphicsSelection(pSimulationState, &selection);

    return errC;
}

bool deinitializeGraphics(foeSimulationState *pSimulationState) {
    return deinitializeGraphicsSelection(pSimulationState, nullptr);
}

} // namespace

int foeGraphicsResourceFunctionalityID() { return FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID; }

auto foeGraphicsResourceRegisterFunctionality() -> std::error_code {
    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foeGraphicsResourceFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
        .pInitializeGraphicsFn = initializeGraphics,
        .pDeinitializeGraphicsFn = deinitializeGraphics,
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

    foeDeregisterFunctionality(foeGraphicsResourceFunctionalityID());

    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceDeregisterFunctionality - Completed deregistering functionality")
}