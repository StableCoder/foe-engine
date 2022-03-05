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

template <typename T>
bool destroyItem(foeSimulationState *pSimulation,
                 foeSimulationStructureType sType,
                 char const *pTypeName) {
    size_t count;

    std::error_code errC = foeSimulationDecrementRefCount(pSimulation, sType, &count);
    if (errC) {
        // Trying to destroy something that doesn't exist? Not optimal
        FOE_LOG(foeGraphicsResource, Warning,
                "Attempted to decrement/destroy {} that doesn't exist - {}", pTypeName,
                errC.message());
        return false;
    } else if (count == 0) {
        T *pLoader;
        errC = foeSimulationReleaseResourceLoader(pSimulation, sType, (void **)&pLoader);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Warning, "Could not release {} to destroy - {}", pTypeName,
                    errC.message());
            return false;
        } else {
            delete pLoader;
        }
    }

    return true;
}

bool destroySelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    bool issues = false;

    // Loaders
    if (pSelection == nullptr || pSelection->meshLoader) {
        issues |= !destroyItem<foeMeshLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        issues |= !destroyItem<foeVertexDescriptorLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            "foeVertexDescriptorLoader");
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        issues |= !destroyItem<foeShaderLoader>(pSimulationState,
                                                FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                                                "foeShaderLoader");
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        issues |= !destroyItem<foeMaterialLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
            "foeMaterialLoader");
    }

    if (pSelection == nullptr || pSelection->imageLoader) {
        issues |= !destroyItem<foeImageLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader");
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

    return !issues;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    std::error_code errC;
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
    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
            .pLoader = new foeImageLoader,
            .pCanProcessCreateInfoFn = foeImageLoader::canProcessCreateInfo,
            .pLoadFn = foeImageLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    reinterpret_cast<foeImageLoader *>(pLoader)->gfxMaintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeImageLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeImageLoader on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, nullptr);
    }
    selection.imageLoader = true;

    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
            .pLoader = new foeMaterialLoader,
            .pCanProcessCreateInfoFn = foeMaterialLoader::canProcessCreateInfo,
            .pLoadFn = foeMaterialLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    reinterpret_cast<foeMaterialLoader *>(pLoader)->gfxMaintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeMaterialLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeMaterialLoader on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, nullptr);
    }
    selection.materialLoader = true;

    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
            .pLoader = new foeShaderLoader,
            .pCanProcessCreateInfoFn = foeShaderLoader::canProcessCreateInfo,
            .pLoadFn = foeShaderLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    reinterpret_cast<foeShaderLoader *>(pLoader)->gfxMaintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeShaderLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeShaderLoader on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, nullptr);
    }
    selection.shaderLoader = true;

    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            .pLoader = new foeVertexDescriptorLoader,
            .pCanProcessCreateInfoFn = foeVertexDescriptorLoader::canProcessCreateInfo,
            .pLoadFn = foeVertexDescriptorLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    reinterpret_cast<foeVertexDescriptorLoader *>(pLoader)->gfxMaintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeVertexDescriptorLoader *)loaderCI.pLoader;
            FOE_LOG(
                foeGraphicsResource, Error,
                "onCreate - Failed to create foeVertexDescriptorLoader on Simulation {} due to {}",
                (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            nullptr);
    }
    selection.vertexDescriptorLoader = true;

    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
            .pLoader = new foeMeshLoader,
            .pCanProcessCreateInfoFn = foeMeshLoader::canProcessCreateInfo,
            .pLoadFn = foeMeshLoader::load,
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    reinterpret_cast<foeMeshLoader *>(pLoader)->gfxMaintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeMeshLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeMeshLoader on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, nullptr);
    }
    selection.meshLoader = true;

CREATE_FAILED:
    if (errC) {
        destroySelection(pSimulationState, &selection);
    }

    return errC;
}

bool destroy(foeSimulationState *pSimulationState) {
    return destroySelection(pSimulationState, nullptr);
}

template <typename T>
bool deinitializeItem(foeSimulationState *pSimulationState,
                      foeSimulationStructureType sType,
                      char const *pTypeName) {
    size_t count;

    std::error_code errC = foeSimulationDecrementInitCount(pSimulationState, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Warning,
                "Failed to decrement {} initialization count on Simulation {} "
                "with error {}",
                pTypeName, (void *)pSimulationState, errC.message());
        return false;
    } else if (count == 0) {
        auto pItem = (T *)foeSimulationGetResourceLoader(pSimulationState, sType);
        pItem->deinitialize();
    }

    return true;
}

bool deinitializeSelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    bool issues = false;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        issues |= !deinitializeItem<foeImageLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader");
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        issues |= !deinitializeItem<foeMaterialLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
            "foeMaterialLoader");
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        issues |= !deinitializeItem<foeShaderLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
            "foeShaderLoader");
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        issues |= !deinitializeItem<foeVertexDescriptorLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            "foeVertexDescriptorLoader");
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        issues |= !deinitializeItem<foeMeshLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
    }

    return true;
}

std::error_code initialize(foeSimulationState *pSimulationState,
                           foeSimulationInitInfo const *pInitInfo) {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Loaders
    errC = foeSimulationIncrementInitCount(
        pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeImageLoader init count due to error: {}", errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.imageLoader = true;
        auto *pLoader = (foeImageLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);
        errC = pLoader->initialize(pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeImageLoader with error: {}", errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeMaterialLoader init count due to error: {}",
                errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.materialLoader = true;

        auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);
        auto *pImagePool = (foeImagePool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

        auto *pLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);
        errC = pLoader->initialize(pShaderPool, pImagePool);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeMaterialLoader with error: {}", errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeShaderLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.shaderLoader = true;
        auto *pLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);
        errC = pLoader->initialize(pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeShaderLoader on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeVertexDescriptorLoader initialization count on Simulation "
                "{} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.shaderLoader = true;
        auto *pLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);

        auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

        errC = pLoader->initialize(pShaderPool);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeVertexDescriptorLoader on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, &count);
    if (errC) {
        FOE_LOG(
            foeGraphicsResource, Error,
            "Failed to increment foeMeshLoader initialization count on Simulation {} with error {}",
            (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    } else if (count == 1) {
        selection.shaderLoader = true;
        auto *pLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);

        errC = pLoader->initialize(pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeMeshLoader on Simulation {} with error {}",
                    (void *)pSimulationState, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitializeSelection(pSimulationState, &selection);

    return errC;
}

bool deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelection(pSimulationState, nullptr);
}

template <typename T>
bool deinitializeGraphicsItem(foeSimulationState *pSimulationState,
                              foeSimulationStructureType sType,
                              char const *pTypeName) {
    size_t count;

    std::error_code errC = foeSimulationDecrementGfxInitCount(pSimulationState, sType, &count);
    if (errC) {
        FOE_LOG(
            foeGraphicsResource, Warning,
            "Failed to decrement {} graphics initialization count on Simulation {} with error {}",
            pTypeName, (void *)pSimulationState, errC.message());
        return false;
    } else if (count == 0) {
        auto *pItem = (T *)foeSimulationGetResourceLoader(pSimulationState, sType);
        pItem->deinitializeGraphics();
    }

    return true;
}

bool deinitializeGraphicsSelection(foeSimulationState *pSimulationState,
                                   TypeSelection const *pSelection) {
    bool issues = false;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        issues |= deinitializeGraphicsItem<foeImageLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader");
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        issues |= deinitializeGraphicsItem<foeMaterialLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
            "foeMaterialLoader");
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        issues |= deinitializeGraphicsItem<foeShaderLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
            "foeShaderLoader");
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        issues |= deinitializeGraphicsItem<foeMeshLoader>(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
    }

    return !issues;
}

template <typename T>
auto initializeGraphicsItem(foeSimulationState *pSimulationState,
                            foeGfxSession gfxSession,
                            foeSimulationStructureType sType,
                            char const *pTypeName) -> std::error_code {
    size_t count;

    auto errC = foeSimulationIncrementGfxInitCount(pSimulationState, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment graphics initialization for {} count on Simulation {} with "
                "error {}",
                pTypeName, (void *)pSimulationState, errC.message());
        return errC;
    } else if (count == 1) {
        auto *pLoader = (T *)foeSimulationGetResourceLoader(pSimulationState, sType);
        errC = pLoader->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed graphics initialization for {} on Simulation {} with error {}",
                    pTypeName, (void *)pSimulationState, errC.message());
            return errC;
        }
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

std::error_code initializeGraphics(foeSimulationState *pSimulationState, foeGfxSession gfxSession) {
    std::error_code errC;
    TypeSelection selection = {};

    // Loaders
    errC = initializeGraphicsItem<foeImageLoader>(pSimulationState, gfxSession,
                                                  FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                                                  "foeImageLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.imageLoader = true;

    errC = initializeGraphicsItem<foeMaterialLoader>(
        pSimulationState, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
        "foeMaterialLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.materialLoader = true;

    errC = initializeGraphicsItem<foeShaderLoader>(
        pSimulationState, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
        "foeShaderLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.shaderLoader = true;

    errC = initializeGraphicsItem<foeMeshLoader>(pSimulationState, gfxSession,
                                                 FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
                                                 "foeMeshLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.meshLoader = true;

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