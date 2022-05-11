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

#include <foe/graphics/resource/registration.h>

#include <foe/graphics/resource/image_loader.hpp>
#include <foe/graphics/resource/material_loader.hpp>
#include <foe/graphics/resource/mesh_loader.hpp>
#include <foe/graphics/resource/shader_loader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_loader.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource_fns.h>
#include <foe/simulation/group_data.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "error_code.hpp"
#include "log.hpp"

namespace {

struct TypeSelection {
    // Loaders
    bool imageLoader;
    bool materialLoader;
    bool shaderLoader;
    bool vertexDescriptorLoader;
    bool meshLoader;
};

template <typename T>
auto destroyItem(foeSimulation *pSimulation,
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

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t errors = 0;

    // Loaders
    if (pSelection == nullptr || pSelection->meshLoader) {
        if (destroyItem<foeMeshLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        if (destroyItem<foeVertexDescriptorLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
                "foeVertexDescriptorLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        if (destroyItem<foeShaderLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, "foeShaderLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        if (destroyItem<foeMaterialLoader>(pSimulation,
                                           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                                           "foeMaterialLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->imageLoader) {
        if (destroyItem<foeImageLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader"))
            ++errors;
    }

    return errors;
}

auto create(foeSimulation *pSimulation) -> std::error_code {
    std::error_code errC;
    TypeSelection selection = {};

    // Loaders
    if (foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, nullptr)) {
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeImageLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeImageLoader on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, nullptr);
    }
    selection.imageLoader = true;

    if (foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, nullptr)) {
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeMaterialLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeMaterialLoader on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, nullptr);
    }
    selection.materialLoader = true;

    if (foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, nullptr)) {
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeShaderLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeShaderLoader on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, nullptr);
    }
    selection.shaderLoader = true;

    if (foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, nullptr)) {
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeVertexDescriptorLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeVertexDescriptorLoader on Simulation {} "
                    "due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, nullptr);
    }
    selection.vertexDescriptorLoader = true;

    if (foeSimulationIncrementRefCount(pSimulation,
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
        errC = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        if (errC) {
            delete (foeMeshLoader *)loaderCI.pLoader;
            FOE_LOG(foeGraphicsResource, Error,
                    "onCreate - Failed to create foeMeshLoader on Simulation {} due to {}",
                    (void *)pSimulation, errC.message());
            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, nullptr);
    }
    selection.meshLoader = true;

CREATE_FAILED:
    if (errC) {
        size_t errors = destroySelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, Warning,
                    "Encountered {} issues destroying after failed creation", errors);
    }

    return errC;
}

size_t destroy(foeSimulation *pSimulation) { return destroySelection(pSimulation, nullptr); }

template <typename T>
auto deinitializeItem(foeSimulation *pSimulation,
                      foeSimulationStructureType sType,
                      char const *pTypeName) -> std::error_code {
    size_t count;

    std::error_code errC = foeSimulationDecrementInitCount(pSimulation, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Warning,
                "Failed to decrement {} initialization count on Simulation {} "
                "with error {}",
                pTypeName, (void *)pSimulation, errC.message());
        return errC;
    } else if (count == 0) {
        auto pItem = (T *)foeSimulationGetResourceLoader(pSimulation, sType);
        pItem->deinitialize();
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

size_t deinitializeSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t errors = 0;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        if (deinitializeItem<foeImageLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        if (deinitializeItem<foeMaterialLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                "foeMaterialLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        if (deinitializeItem<foeShaderLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, "foeShaderLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        if (deinitializeItem<foeVertexDescriptorLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
                "foeVertexDescriptorLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        if (deinitializeItem<foeMeshLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader"))
            ++errors;
    }

    return errors;
}

std::error_code initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo) {
    std::error_code errC;
    size_t count;
    TypeSelection selection = {};

    // Loaders
    errC = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeImageLoader init count due to error: {}", errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.imageLoader = true;
    if (count == 1) {
        auto *pLoader = (foeImageLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);

        errC = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeImageLoader with error: {}", errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeMaterialLoader init count due to error: {}",
                errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.materialLoader = true;
    if (count == 1) {
        auto *pLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);

        errC = pLoader->initialize(pSimulation->resourcePool);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeMaterialLoader with error: {}", errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeShaderLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.shaderLoader = true;
    if (count == 1) {
        auto *pLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);

        errC = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeShaderLoader on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeVertexDescriptorLoader initialization count on Simulation "
                "{} with error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.vertexDescriptorLoader = true;
    if (count == 1) {
        auto *pLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);

        errC = pLoader->initialize(pSimulation->resourcePool);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeVertexDescriptorLoader on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

    errC = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment foeMeshLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, errC.message());
        goto INITIALIZATION_FAILED;
    }
    selection.meshLoader = true;
    if (count == 1) {
        auto *pLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);

        errC = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed to initialize foeMeshLoader on Simulation {} with error {}",
                    (void *)pSimulation, errC.message());
            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, Warning,
                    "Encountered {} issues while deinitializing after failed initialization",
                    errors);
    }

    return errC;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
}

template <typename T>
auto deinitializeGraphicsItem(foeSimulation *pSimulation,
                              foeSimulationStructureType sType,
                              char const *pTypeName) -> std::error_code {
    size_t count;

    std::error_code errC = foeSimulationDecrementGfxInitCount(pSimulation, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Warning,
                "Failed to decrement {} graphics initialization count on Simulation {} with "
                "error {}",
                pTypeName, (void *)pSimulation, errC.message());
        return errC;
    } else if (count == 0) {
        auto *pItem = (T *)foeSimulationGetResourceLoader(pSimulation, sType);
        pItem->deinitializeGraphics();
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

size_t deinitializeGraphicsSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t errors = 0;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        if (deinitializeGraphicsItem<foeImageLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        if (deinitializeGraphicsItem<foeMaterialLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
                "foeMaterialLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        if (deinitializeGraphicsItem<foeShaderLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, "foeShaderLoader"))
            ++errors;
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        if (deinitializeGraphicsItem<foeMeshLoader>(
                pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader"))
            ++errors;
    }

    return errors;
}

template <typename T>
auto initializeGraphicsItem(foeSimulation *pSimulation,
                            foeGfxSession gfxSession,
                            foeSimulationStructureType sType,
                            char const *pTypeName) -> std::error_code {
    size_t count;

    auto errC = foeSimulationIncrementGfxInitCount(pSimulation, sType, &count);
    if (errC) {
        FOE_LOG(foeGraphicsResource, Error,
                "Failed to increment graphics initialization for {} count on Simulation {} with "
                "error {}",
                pTypeName, (void *)pSimulation, errC.message());
        return errC;
    } else if (count == 1) {
        auto *pLoader = (T *)foeSimulationGetResourceLoader(pSimulation, sType);
        errC = pLoader->initializeGraphics(gfxSession);
        if (errC) {
            FOE_LOG(foeGraphicsResource, Error,
                    "Failed graphics initialization for {} on Simulation {} with error {}",
                    pTypeName, (void *)pSimulation, errC.message());
            return errC;
        }
    }

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

std::error_code initializeGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) {
    std::error_code errC;
    TypeSelection selection = {};

    // Loaders
    errC = initializeGraphicsItem<foeImageLoader>(pSimulation, gfxSession,
                                                  FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
                                                  "foeImageLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.imageLoader = true;

    errC = initializeGraphicsItem<foeMaterialLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
        "foeMaterialLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.materialLoader = true;

    errC = initializeGraphicsItem<foeShaderLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
        "foeShaderLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.shaderLoader = true;

    errC = initializeGraphicsItem<foeMeshLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
    if (errC)
        goto INITIALIZATION_FAILED;
    selection.meshLoader = true;

INITIALIZATION_FAILED:
    if (errC) {
        size_t errors = deinitializeGraphicsSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, Warning,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return errC;
}

size_t deinitializeGraphics(foeSimulation *pSimulation) {
    return deinitializeGraphicsSelection(pSimulation, nullptr);
}

} // namespace

int foeGraphicsResourceFunctionalityID() { return FOE_GRAPHICS_RESOURCE_FUNCTIONALITY_ID; }

extern "C" foeErrorCode foeGraphicsResourceRegisterFunctionality() {
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

    return foeToErrorCode(errC);
}

extern "C" void foeGraphicsResourceDeregisterFunctionality() {
    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeGraphicsResourceFunctionalityID());

    FOE_LOG(foeGraphicsResource, Verbose,
            "foeGraphicsResourceDeregisterFunctionality - Completed deregistering functionality")
}