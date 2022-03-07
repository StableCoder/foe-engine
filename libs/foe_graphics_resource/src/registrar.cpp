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
            it.pLoadFn(it.pLoader, pImage, pLocalCreateInfo, pPostLoadFn);
            return;
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
auto destroyItem(foeSimulationState *pSimulation,
                 foeSimulationStructureType sType,
                 char const *pTypeName) -> std::error_code {
    size_t count;

    std::error_code errC = foeSimulationDecrementRefCount(pSimulation, sType, &count);
    if (errC) {
        // Trying to destroy something that doesn't exist? Not optimal
        FOE_LOG(foeGraphicsResource, Warning,
                "Attempted to decrement/destroy {} that doesn't exist - {}", pTypeName,
                errC.message());
        return errC;
    } else if (count == 0) {
        T *pLoader;
        errC = foeSimulationReleaseResourceLoader(pSimulation, sType, (void **)&pLoader);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Warning, "Could not release {} to destroy - {}", pTypeName,
                    errC.message());
            return errC;
        } else {
            delete pLoader;
        }
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

template <typename T>
auto destroyItem2(foeSimulationState *pSimulation,
                  foeSimulationStructureType sType,
                  char const *pTypeName) -> std::error_code {
    size_t count;

    std::error_code errC = foeSimulationDecrementRefCount(pSimulation, sType, &count);
    if (errC) {
        // Trying to destroy something that doesn't exist? Not optimal
        FOE_LOG(foeGraphicsResource, Warning,
                "Attempted to decrement/destroy {} that doesn't exist - {}", pTypeName,
                errC.message());
        return errC;
    } else if (count == 0) {
        T *pLoader;
        errC = foeSimulationReleaseResourcePool(pSimulation, sType, (void **)&pLoader);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Warning, "Could not release {} to destroy - {}", pTypeName,
                    errC.message());
            return errC;
        } else {
            delete pLoader;
        }
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

size_t destroySelection(foeSimulationState *pSimulationState, TypeSelection const *pSelection) {
    size_t errors = 0;

    // Loaders
    if (pSelection == nullptr || pSelection->meshLoader) {
        if (destroyItem<foeMeshLoader>(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
                                       "foeMeshLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        if (destroyItem<foeVertexDescriptorLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
                "foeVertexDescriptorLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        if (destroyItem<foeShaderLoader>(pSimulationState,
                                         FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                                         "foeShaderLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        if (destroyItem<foeMaterialLoader>(pSimulationState,
                                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                                           "foeMaterialLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->imageLoader) {
        if (destroyItem<foeImageLoader>(pSimulationState,
                                        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                                        "foeImageLoader"))
            ++errors;
    }

    // Resources
    if (pSelection == nullptr || pSelection->meshResources) {
        if (destroyItem2<foeMeshPool>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL, "foeMeshPool"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorResources) {
        if (destroyItem2<foeVertexDescriptorPool>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL,
                "foeVertexDescriptorPool"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderResources) {
        if (destroyItem2<foeShaderPool>(pSimulationState,
                                        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL,
                                        "foeShaderPool"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialResources) {
        if (destroyItem2<foeMaterialPool>(pSimulationState,
                                          FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL,
                                          "foeMaterialPool"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->imageResources) {
        if (destroyItem2<foeImagePool>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL, "foeImagePool"))
            ++errors;
    }

    return errors;
}

auto create(foeSimulationState *pSimulationState) -> std::error_code {
    std::error_code errC;
    TypeSelection selection = {};

    // Resources
    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationResourcePoolData createInfo{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL,
            .pResourcePool = new foeImagePool{foeResourceFns{
                .pImportContext = &pSimulationState->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulationState,
                .pLoadFn = imageLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeImagePool *)createInfo.pResourcePool;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeImagePool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL, nullptr);
    }
    selection.imageResources = true;

    if (foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationResourcePoolData createInfo{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL,
            .pResourcePool = new foeMaterialPool{foeResourceFns{
                .pImportContext = &pSimulationState->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulationState,
                .pLoadFn = materialLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeMaterialPool *)createInfo.pResourcePool;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeMaterialPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL, nullptr);
    }
    selection.materialResources = true;

    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationResourcePoolData createInfo{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL,
            .pResourcePool = new foeShaderPool{foeResourceFns{
                .pImportContext = &pSimulationState->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulationState,
                .pLoadFn = shaderLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeShaderPool *)createInfo.pResourcePool;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeShaderPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL, nullptr);
    }
    selection.shaderResources = true;

    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL,
                                       nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationResourcePoolData createInfo{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL,
            .pResourcePool = new foeVertexDescriptorPool{foeResourceFns{
                .pImportContext = &pSimulationState->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulationState,
                .pLoadFn = vertexDescriptorLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeVertexDescriptorPool *)createInfo.pResourcePool;
            FOE_LOG(
                foeGraphicsResource, Error,
                "onCreate - Failed to create foeVertexDescriptorPool on Simulation {} due to {}",
                (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL, nullptr);
    }
    selection.vertexDescriptorResources = true;

    if (foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL, nullptr)) {
        // Couldn't incement it, doesn't exist yet
        foeSimulationResourcePoolData createInfo{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL,
            .pResourcePool = new foeMeshPool{foeResourceFns{
                .pImportContext = &pSimulationState->groupData,
                .pImportFn = importFn,
                .pLoadContext = pSimulationState,
                .pLoadFn = meshLoadFn,
            }},
        };
        errC = foeSimulationInsertResourcePool(pSimulationState, &createInfo);
        if (errC) {
            delete (foeMeshPool *)createInfo.pResourcePool;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeMeshPool on Simulation {} due to {}",
                    (void *)pSimulationState, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulationState,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL, nullptr);
    }
    selection.meshResources = true;

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
                [](void *pLoader) {
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
                [](void *pLoader) {
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
                [](void *pLoader) {
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
                [](void *pLoader) {
                    reinterpret_cast<foeVertexDescriptorLoader *>(pLoader)->gfxMaintenance();
                },
        };
        errC = foeSimulationInsertResourceLoader(pSimulationState, &loaderCI);
        if (errC) {
            delete (foeVertexDescriptorLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeVertexDescriptorLoader on Simulation {} "
                    "due to {}",
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
                [](void *pLoader) { reinterpret_cast<foeMeshLoader *>(pLoader)->gfxMaintenance(); },
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
        size_t errors = destroySelection(pSimulationState, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, Warning,
                    "Encountered {} issues destroying after failed creation", errors);
    }

    return errC;
}

size_t destroy(foeSimulationState *pSimulationState) {
    return destroySelection(pSimulationState, nullptr);
}

template <typename T>
auto deinitializeItem(foeSimulationState *pSimulationState,
                      foeSimulationStructureType sType,
                      char const *pTypeName) -> std::error_code {
    size_t count;

    std::error_code errC = foeSimulationDecrementInitCount(pSimulationState, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Warning,
                "Failed to decrement {} initialization count on Simulation {} "
                "with error {}",
                pTypeName, (void *)pSimulationState, errC.message());
        return errC;
    } else if (count == 0) {
        auto pItem = (T *)foeSimulationGetResourceLoader(pSimulationState, sType);
        pItem->deinitialize();
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

size_t deinitializeSelection(foeSimulationState *pSimulationState,
                             TypeSelection const *pSelection) {
    size_t errors = 0;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        if (deinitializeItem<foeImageLoader>(pSimulationState,
                                             FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                                             "foeImageLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        if (deinitializeItem<foeMaterialLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                "foeMaterialLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        if (deinitializeItem<foeShaderLoader>(pSimulationState,
                                              FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                                              "foeShaderLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        if (deinitializeItem<foeVertexDescriptorLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
                "foeVertexDescriptorLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        if (deinitializeItem<foeMeshLoader>(pSimulationState,
                                            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
                                            "foeMeshLoader"))
            ++errors;
    }

    return errors;
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
    }
    selection.imageLoader = true;
    if (count == 1) {
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
    }
    selection.materialLoader = true;
    if (count == 1) {
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
    }
    selection.shaderLoader = true;
    if (count == 1) {
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
                "{} with error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.vertexDescriptorLoader = true;
    if (count == 1) {
        auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

        auto *pLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);
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
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeMeshLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulationState, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.meshLoader = true;
    if (count == 1) {
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
    if (errC) {
        size_t errors = deinitializeSelection(pSimulationState, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, Warning,
                    "Encountered {} issues while deinitializing after failed initialization",
                    errors);
    }

    return errC;
}

size_t deinitialize(foeSimulationState *pSimulationState) {
    return deinitializeSelection(pSimulationState, nullptr);
}

template <typename T>
auto deinitializeGraphicsItem(foeSimulationState *pSimulationState,
                              foeSimulationStructureType sType,
                              char const *pTypeName) -> std::error_code {
    size_t count;

    std::error_code errC = foeSimulationDecrementGfxInitCount(pSimulationState, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Warning,
                "Failed to decrement {} graphics initialization count on Simulation {} with "
                "error {}",
                pTypeName, (void *)pSimulationState, errC.message());
        return errC;
    } else if (count == 0) {
        auto *pItem = (T *)foeSimulationGetResourceLoader(pSimulationState, sType);
        pItem->deinitializeGraphics();
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

size_t deinitializeGraphicsSelection(foeSimulationState *pSimulationState,
                                     TypeSelection const *pSelection) {
    size_t errors = 0;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        if (deinitializeGraphicsItem<foeImageLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                "foeImageLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        if (deinitializeGraphicsItem<foeMaterialLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                "foeMaterialLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        if (deinitializeGraphicsItem<foeShaderLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                "foeShaderLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        if (deinitializeGraphicsItem<foeMeshLoader>(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
                "foeMeshLoader"))
            ++errors;
    }

    return errors;
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
    if (errC) {
        size_t errors = deinitializeGraphicsSelection(pSimulationState, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, Warning,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return errC;
}

size_t deinitializeGraphics(foeSimulationState *pSimulationState) {
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