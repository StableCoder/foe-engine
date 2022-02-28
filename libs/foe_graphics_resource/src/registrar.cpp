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

struct Initialized {
    bool image;
    bool material;
    bool shader;
    bool vertexDescriptor;
    bool mesh;
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
    Initialized found{};

    // Find any matching pool types that have been created already, and if so, increment the
    // reference count
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL) {
            ++pPool->refCount;
            found.image = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL) {
            ++pPool->refCount;
            found.material = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL) {
            ++pPool->refCount;
            found.shader = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL) {
            ++pPool->refCount;
            found.vertexDescriptor = true;
        }
        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL) {
            ++pPool->refCount;
            found.mesh = true;
        }
    }

    // For any resource pools not yet created, do so now
    if (!found.image) {
        auto *pPool = new foeImagePool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = imageLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }
    if (!found.material) {
        auto *pPool = new foeMaterialPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = materialLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }
    if (!found.shader) {
        auto *pPool = new foeShaderPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = shaderLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }
    if (!found.vertexDescriptor) {
        auto *pPool = new foeVertexDescriptorPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = vertexDescriptorLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
    }
    if (!found.mesh) {
        auto *pPool = new foeMeshPool{foeResourceFns{
            .pImportContext = &pSimulationState->groupData,
            .pImportFn = importFn,
            .pLoadContext = pSimulationState,
            .pLoadFn = meshLoadFn,
        }};
        ++pPool->refCount;
        pSimulationState->resourcePools.emplace_back(pPool);
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
            .pGfxMaintenanceFn =
                [](foeResourceLoaderBase *pLoader) {
                    auto *pImageLoader = reinterpret_cast<foeImageLoader *>(pLoader);
                    pImageLoader->gfxMaintenance();
                },
        });
    }

    if (auto *pLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);
        pLoader != nullptr) {
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

    if (auto *pLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);
        pLoader != nullptr) {
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

    if (auto *pLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);
        pLoader != nullptr) {
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

    if (auto *pLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);
        pLoader != nullptr) {
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

#define DESTROY_LOADER(X, Y)                                                                       \
    if (it.pLoader->sType == Y && --it.pLoader->refCount == 0) {                                   \
        delete (X *)it.pLoader;                                                                    \
        it.pLoader = nullptr;                                                                      \
        continue;                                                                                  \
    }

void onDestroy(foeSimulationState *pSimulationState) {
    // Loaders
    for (auto &it : pSimulationState->resourceLoaders) {
        if (it.pLoader == nullptr)
            continue;

        DESTROY_LOADER(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER)
        DESTROY_LOADER(foeVertexDescriptorLoader,
                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER)
        DESTROY_LOADER(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER)
        DESTROY_LOADER(foeMaterialLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER)
        DESTROY_LOADER(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER)
    }

    // Resources
    for (auto &pPool : pSimulationState->resourcePools) {
        if (pPool == nullptr)
            continue;

        if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_POOL) {
            auto *pTemp = (foeMeshPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_POOL) {
            auto *pTemp = (foeVertexDescriptorPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL) {
            auto *pTemp = (foeShaderPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_POOL) {
            auto *pTemp = (foeMaterialPool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        } else if (pPool->sType == FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL) {
            auto *pTemp = (foeImagePool *)pPool;
            if (--pTemp->refCount == 0) {
                delete pTemp;
                pPool = nullptr;
            }
        }
    }
}

#define DEINITIALIZE_LOADER(X, Y)                                                                  \
    if (auto *pLoader = (X *)foeSimulationGetResourceLoader(pSimulationState, Y);                  \
        pLoader != nullptr && --pLoader->initCount == 0) {                                         \
        pLoader->deinitialize();                                                                   \
    }

void deinitialize(foeSimulationState const *pSimulationState, Initialized const &initialized) {
    // Loaders
    if (initialized.image)
        DEINITIALIZE_LOADER(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);

    if (initialized.material)
        DEINITIALIZE_LOADER(foeMaterialLoader,
                            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);

    if (initialized.shader)
        DEINITIALIZE_LOADER(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);

    if (initialized.vertexDescriptor)
        DEINITIALIZE_LOADER(foeVertexDescriptorLoader,
                            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);

    if (initialized.mesh)
        DEINITIALIZE_LOADER(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);
}

std::error_code onInitialization(foeSimulationState const *pSimulationState,
                                 foeSimulationInitInfo const *pInitInfo) {
    std::error_code errC;
    Initialized initialized{};

    // Loaders
    if (auto *pImageLoader = (foeImageLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);
        pImageLoader) {
        ++pImageLoader->initCount;
        if (!pImageLoader->initialized()) {
            errC = pImageLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.image = true;
        }
    }

    if (auto *pMaterialLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);
        pMaterialLoader) {
        ++pMaterialLoader->initCount;
        if (!pMaterialLoader->initialized()) {
            auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);
            auto *pImagePool = (foeImagePool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL);

            errC = pMaterialLoader->initialize(pShaderPool, pImagePool);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.material = true;
        }
    }

    if (auto *pShaderLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);
        pShaderLoader) {
        ++pShaderLoader->initCount;
        if (!pShaderLoader->initialized()) {
            errC = pShaderLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.shader = true;
        }
    }

    if (auto *pVertexDescriptorLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);
        pVertexDescriptorLoader) {
        ++pVertexDescriptorLoader->initCount;
        if (!pVertexDescriptorLoader->initialized()) {
            auto *pShaderPool = (foeShaderPool *)foeSimulationGetResourcePool(
                pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_POOL);

            errC = pVertexDescriptorLoader->initialize(pShaderPool);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.vertexDescriptor = true;
        }
    }

    if (auto *pMeshLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulationState, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);
        pMeshLoader) {
        ++pMeshLoader->initCount;
        if (!pMeshLoader->initialized()) {
            errC = pMeshLoader->initialize(pInitInfo->externalFileSearchFn);
            if (errC)
                goto INITIALIZATION_FAILED;
            initialized.mesh = true;
        }
    }

INITIALIZATION_FAILED:
    if (errC)
        deinitialize(pSimulationState, initialized);

    return errC;
}

void onDeinitialization(foeSimulationState const *pSimulationState) {
    // Loaders
    DEINITIALIZE_LOADER(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);
    DEINITIALIZE_LOADER(foeMaterialLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);
    DEINITIALIZE_LOADER(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);
    DEINITIALIZE_LOADER(foeVertexDescriptorLoader,
                        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);
    DEINITIALIZE_LOADER(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);
}

#define DEINITIALIZE_LOADER_GRAPHICS(X, Y)                                                         \
    if (auto *pLoader = (X *)foeSimulationGetResourceLoader(pSimulationState, Y);                  \
        pLoader != nullptr && --pLoader->gfxInitCount == 0) {                                      \
        pLoader->deinitializeGraphics();                                                           \
    }

void deinitializeGraphics(foeSimulationState const *pSimulationState,
                          Initialized const &initialized) {
    // Loaders
    if (initialized.image)
        DEINITIALIZE_LOADER_GRAPHICS(foeImageLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER)
    if (initialized.material)
        DEINITIALIZE_LOADER_GRAPHICS(foeMaterialLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER)
    if (initialized.shader)
        DEINITIALIZE_LOADER_GRAPHICS(foeShaderLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER)
    if (initialized.mesh)
        DEINITIALIZE_LOADER_GRAPHICS(foeMeshLoader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER)
}

#define INITIALIZE_LOADER_GRAPHICS(X, Y, Z)                                                        \
    if (auto *pLoader = (X *)foeSimulationGetResourceLoader(pSimulationState, Y);                  \
        pLoader != nullptr) {                                                                      \
        ++pLoader->gfxInitCount;                                                                   \
        initialized.Z = true;                                                                      \
        if (!pLoader->initializedGraphics()) {                                                     \
            errC = pLoader->initializeGraphics(gfxSession);                                        \
            if (errC)                                                                              \
                goto INITIALIZATION_FAILED;                                                        \
        }                                                                                          \
    }

std::error_code onGfxInitialization(foeSimulationState const *pSimulationState,
                                    foeGfxSession gfxSession) {
    std::error_code errC;
    Initialized initialized{};

    // Loaders
    INITIALIZE_LOADER_GRAPHICS(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                               image);

    INITIALIZE_LOADER_GRAPHICS(foeMaterialLoader,
                               FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, material);

    INITIALIZE_LOADER_GRAPHICS(foeShaderLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
                               shader);

    INITIALIZE_LOADER_GRAPHICS(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
                               mesh);

INITIALIZATION_FAILED:
    if (errC)
        deinitializeGraphics(pSimulationState, initialized);

    return errC;
}

void onGfxDeinitialization(foeSimulationState const *pSimulationState) {
    // Loaders
    DEINITIALIZE_LOADER_GRAPHICS(foeImageLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER)
    DEINITIALIZE_LOADER_GRAPHICS(foeMaterialLoader,
                                 FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER)
    DEINITIALIZE_LOADER_GRAPHICS(foeShaderLoader,
                                 FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER)
    DEINITIALIZE_LOADER_GRAPHICS(foeMeshLoader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER)
}

} // namespace

int foeGraphicsResourceFunctionalityID() { return FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID; }

auto foeGraphicsResourceRegisterFunctionality() -> std::error_code {
    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceRegisterFunctionality - Starting to register functionality")

    auto errC = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foeGraphicsResourceFunctionalityID(),
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
        .id = foeGraphicsResourceFunctionalityID(),
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