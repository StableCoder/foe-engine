// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/registration.h>

#include <foe/graphics/resource/type_defs.h>
#include <foe/resource/pool.h>
#include <foe/resource/resource_fns.h>
#include <foe/simulation/group_data.hpp>
#include <foe/simulation/registration.hpp>
#include <foe/simulation/simulation.hpp>

#include "image_loader.hpp"
#include "material_loader.hpp"
#include "mesh_loader.hpp"
#include "shader_loader.hpp"
#include "vertex_descriptor_loader.hpp"

#include "log.hpp"
#include "result.h"

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
foeResultSet destroyItem(foeSimulation *pSimulation,
                         foeSimulationStructureType sType,
                         char const *pTypeName) {
    size_t count;

    foeResultSet result = foeSimulationDecrementRefCount(pSimulation, sType, &count);
    if (result.value != FOE_SUCCESS) {
        // Trying to destroy something that doesn't exist? Not optimal
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                "Attempted to decrement/destroy {} that doesn't exist - {}", pTypeName, buffer);

        return result;
    } else if (count == 0) {
        T *pLoader;
        result = foeSimulationReleaseResourceLoader(pSimulation, sType, (void **)&pLoader);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                    "Could not release {} to destroy - {}", pTypeName, buffer);

            return result;
        } else {
            delete pLoader;
        }
    }

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

size_t destroySelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t errors = 0;
    foeResultSet result;

    // Loaders
    if (pSelection == nullptr || pSelection->meshLoader) {
        result = destroyItem<foeMeshLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        result = destroyItem<foeVertexDescriptorLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            "foeVertexDescriptorLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        result = destroyItem<foeShaderLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, "foeShaderLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        result = destroyItem<foeMaterialLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, "foeMaterialLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->imageLoader) {
        result = destroyItem<foeImageLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    return errors;
}

foeResultSet create(foeSimulation *pSimulation) {
    foeResultSet result;
    TypeSelection selection = {};

    // Loaders
    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't increment it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
            .pLoader = new (std::nothrow) foeImageLoader,
            .pCanProcessCreateInfoFn = foeImageLoader::canProcessCreateInfo,
            .pLoadFn = foeImageLoader::load,
            .pGfxMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeImageLoader *>(pLoader)->gfxMaintenance();
                },
        };

        if (loaderCI.pLoader != nullptr)
            result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        else
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_OUT_OF_MEMORY);

        if (result.value != FOE_SUCCESS) {
            delete (foeImageLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeImageLoader on Simulation {} due to {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, nullptr);
    }
    selection.imageLoader = true;

    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't increment it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
            .pLoader = new (std::nothrow) foeMaterialLoader,
            .pCanProcessCreateInfoFn = foeMaterialLoader::canProcessCreateInfo,
            .pLoadFn = foeMaterialLoader::load,
            .pGfxMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeMaterialLoader *>(pLoader)->gfxMaintenance();
                },
        };

        if (loaderCI.pLoader != nullptr)
            result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        else
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_OUT_OF_MEMORY);

        if (result.value != FOE_SUCCESS) {
            delete (foeMaterialLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeMaterialLoader on Simulation {} due to {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, nullptr);
    }
    selection.materialLoader = true;

    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't increment it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
            .pLoader = new (std::nothrow) foeShaderLoader,
            .pCanProcessCreateInfoFn = foeShaderLoader::canProcessCreateInfo,
            .pLoadFn = foeShaderLoader::load,
            .pGfxMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeShaderLoader *>(pLoader)->gfxMaintenance();
                },
        };

        if (loaderCI.pLoader != nullptr)
            result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        else
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_OUT_OF_MEMORY);

        if (result.value != FOE_SUCCESS) {
            delete (foeShaderLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeShaderLoader on Simulation {} due to {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, nullptr);
    }
    selection.shaderLoader = true;

    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't increment it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            .pLoader = new (std::nothrow) foeVertexDescriptorLoader,
            .pCanProcessCreateInfoFn = foeVertexDescriptorLoader::canProcessCreateInfo,
            .pLoadFn = foeVertexDescriptorLoader::load,
            .pGfxMaintenanceFn =
                [](void *pLoader) {
                    reinterpret_cast<foeVertexDescriptorLoader *>(pLoader)->gfxMaintenance();
                },
        };

        if (loaderCI.pLoader != nullptr)
            result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        else
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_OUT_OF_MEMORY);

        if (result.value != FOE_SUCCESS) {
            delete (foeVertexDescriptorLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeVertexDescriptorLoader on Simulation {} "
                    "due to {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, nullptr);
    }
    selection.vertexDescriptorLoader = true;

    result = foeSimulationIncrementRefCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, nullptr);
    if (result.value != FOE_SUCCESS) {
        // Couldn't increment it, doesn't exist yet
        foeSimulationLoaderData loaderCI{
            .sType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER,
            .pLoader = new (std::nothrow) foeMeshLoader,
            .pCanProcessCreateInfoFn = foeMeshLoader::canProcessCreateInfo,
            .pLoadFn = foeMeshLoader::load,
            .pGfxMaintenanceFn =
                [](void *pLoader) { reinterpret_cast<foeMeshLoader *>(pLoader)->gfxMaintenance(); },
        };

        if (loaderCI.pLoader != nullptr)
            result = foeSimulationInsertResourceLoader(pSimulation, &loaderCI);
        else
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_OUT_OF_MEMORY);

        if (result.value != FOE_SUCCESS) {
            delete (foeMeshLoader *)loaderCI.pLoader;

            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "onCreate - Failed to create foeMeshLoader on Simulation {} due to {}",
                    (void *)pSimulation, buffer);

            goto CREATE_FAILED;
        }
        foeSimulationIncrementRefCount(pSimulation,
                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, nullptr);
    }
    selection.meshLoader = true;

CREATE_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = destroySelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues destroying after failed creation", errors);
    }

    return result;
}

size_t destroy(foeSimulation *pSimulation) { return destroySelection(pSimulation, nullptr); }

template <typename T>
foeResultSet deinitializeItem(foeSimulation *pSimulation,
                              foeSimulationStructureType sType,
                              char const *pTypeName) {
    size_t count;

    foeResultSet result = foeSimulationDecrementInitCount(pSimulation, sType, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                "Failed to decrement {} initialization count on Simulation {} "
                "with error {}",
                pTypeName, (void *)pSimulation, buffer);

        return result;
    } else if (count == 0) {
        auto pItem = (T *)foeSimulationGetResourceLoader(pSimulation, sType);
        pItem->deinitialize();
    }

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

size_t deinitializeSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t errors = 0;
    foeResultSet result;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        result = deinitializeItem<foeImageLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        result = deinitializeItem<foeMaterialLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, "foeMaterialLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        result = deinitializeItem<foeShaderLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, "foeShaderLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->vertexDescriptorLoader) {
        result = deinitializeItem<foeVertexDescriptorLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER,
            "foeVertexDescriptorLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        result = deinitializeItem<foeMeshLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    return errors;
}

foeResultSet initialize(foeSimulation *pSimulation, foeSimulationInitInfo const *pInitInfo) {
    foeResultSet result;
    size_t count;
    TypeSelection selection = {};

    // Loaders
    result = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeImageLoader init count due to error: {}", buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.imageLoader = true;
    if (count == 1) {
        auto *pLoader = (foeImageLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeImageLoader with error: {}", buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeMaterialLoader init count due to error: {}", buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.materialLoader = true;
    if (count == 1) {
        auto *pLoader = (foeMaterialLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeMaterialLoader with error: {}", buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeShaderLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.shaderLoader = true;
    if (count == 1) {
        auto *pLoader = (foeShaderLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeShaderLoader on Simulation {} with error {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeVertexDescriptorLoader initialization count on Simulation "
                "{} with error {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.vertexDescriptorLoader = true;
    if (count == 1) {
        auto *pLoader = (foeVertexDescriptorLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeVertexDescriptorLoader on Simulation {} with error {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

    result = foeSimulationIncrementInitCount(
        pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to increment foeMeshLoader initialization count on Simulation {} with "
                "error {}",
                (void *)pSimulation, buffer);

        goto INITIALIZATION_FAILED;
    }
    selection.meshLoader = true;
    if (count == 1) {
        auto *pLoader = (foeMeshLoader *)foeSimulationGetResourceLoader(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER);

        result = pLoader->initialize(pSimulation->resourcePool, pInitInfo->externalFileSearchFn);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "Failed to initialize foeMeshLoader on Simulation {} with error {}",
                    (void *)pSimulation, buffer);

            goto INITIALIZATION_FAILED;
        }
    }

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues while deinitializing after failed initialization",
                    errors);
    }

    return result;
}

size_t deinitialize(foeSimulation *pSimulation) {
    return deinitializeSelection(pSimulation, nullptr);
}

template <typename T>
foeResultSet deinitializeGraphicsItem(foeSimulation *pSimulation,
                                      foeSimulationStructureType sType,
                                      char const *pTypeName) {
    size_t count;

    foeResultSet result = foeSimulationDecrementGfxInitCount(pSimulation, sType, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                "Failed to decrement {} graphics initialization count on Simulation {} with "
                "error {}",
                pTypeName, (void *)pSimulation, buffer);

        return result;
    } else if (count == 0) {
        auto *pItem = (T *)foeSimulationGetResourceLoader(pSimulation, sType);
        pItem->deinitializeGraphics();
    }

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

size_t deinitializeGraphicsSelection(foeSimulation *pSimulation, TypeSelection const *pSelection) {
    size_t errors = 0;
    foeResultSet result;

    // Loaders
    if (pSelection == nullptr || pSelection->imageLoader) {
        result = deinitializeGraphicsItem<foeImageLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER, "foeImageLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->materialLoader) {
        result = deinitializeGraphicsItem<foeMaterialLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER, "foeMaterialLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->shaderLoader) {
        result = deinitializeGraphicsItem<foeShaderLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER, "foeShaderLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    if (pSelection == nullptr || pSelection->meshLoader) {
        result = deinitializeGraphicsItem<foeMeshLoader>(
            pSimulation, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
        if (result.value != FOE_SUCCESS)
            ++errors;
    }

    return errors;
}

template <typename T>
foeResultSet initializeGraphicsItem(foeSimulation *pSimulation,
                                    foeGfxSession gfxSession,
                                    foeSimulationStructureType sType,
                                    char const *pTypeName) {
    size_t count;

    foeResultSet result = foeSimulationIncrementGfxInitCount(pSimulation, sType, &count);
    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "Failed to increment graphics initialization for {} count on Simulation {} with "
                "error {}",
                pTypeName, (void *)pSimulation, buffer);

        return result;
    } else if (count == 1) {
        auto *pLoader = (T *)foeSimulationGetResourceLoader(pSimulation, sType);
        result = pLoader->initializeGraphics(gfxSession);
        if (result.value != FOE_SUCCESS) {
            char buffer[FOE_MAX_RESULT_STRING_SIZE];
            result.toString(result.value, buffer);
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                    "Failed graphics initialization for {} on Simulation {} with error {}",
                    pTypeName, (void *)pSimulation, buffer);

            return result;
        }
    }

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

foeResultSet initializeGraphics(foeSimulation *pSimulation, foeGfxSession gfxSession) {
    foeResultSet result;
    TypeSelection selection = {};

    // Loaders
    result = initializeGraphicsItem<foeImageLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_LOADER,
        "foeImageLoader");
    if (result.value != FOE_SUCCESS)
        goto INITIALIZATION_FAILED;
    selection.imageLoader = true;

    result = initializeGraphicsItem<foeMaterialLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_LOADER,
        "foeMaterialLoader");
    if (result.value != FOE_SUCCESS)
        goto INITIALIZATION_FAILED;
    selection.materialLoader = true;

    result = initializeGraphicsItem<foeShaderLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_LOADER,
        "foeShaderLoader");
    if (result.value != FOE_SUCCESS)
        goto INITIALIZATION_FAILED;
    selection.shaderLoader = true;

    result = initializeGraphicsItem<foeMeshLoader>(
        pSimulation, gfxSession, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MESH_LOADER, "foeMeshLoader");
    if (result.value != FOE_SUCCESS)
        goto INITIALIZATION_FAILED;
    selection.meshLoader = true;

INITIALIZATION_FAILED:
    if (result.value != FOE_SUCCESS) {
        size_t errors = deinitializeGraphicsSelection(pSimulation, &selection);
        if (errors > 0)
            FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_WARNING,
                    "Encountered {} issues deinitializing graphics after failed initialization",
                    errors);
    }

    return result;
}

size_t deinitializeGraphics(foeSimulation *pSimulation) {
    return deinitializeGraphicsSelection(pSimulation, nullptr);
}

} // namespace

int foeGraphicsResourceFunctionalityID() { return FOE_GRAPHICS_RESOURCE_LIBRARY_ID; }

extern "C" foeResultSet foeGraphicsResourceRegisterFunctionality() {
    FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_VERBOSE,
            "foeGraphicsResourceRegisterFunctionality - Starting to register functionality")

    foeResultSet result = foeRegisterFunctionality(foeSimulationFunctionalty{
        .id = foeGraphicsResourceFunctionalityID(),
        .pCreateFn = create,
        .pDestroyFn = destroy,
        .pInitializeFn = initialize,
        .pDeinitializeFn = deinitialize,
        .pInitializeGraphicsFn = initializeGraphics,
        .pDeinitializeGraphicsFn = deinitializeGraphics,
    });

    if (result.value != FOE_SUCCESS) {
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeGraphicsResourceRegisterFunctionality - Failed registering functionality: {}",
                buffer)
    } else {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_VERBOSE,
                "foeGraphicsResourceRegisterFunctionality - Completed registering functionality")
    }

    return result;
}

extern "C" void foeGraphicsResourceDeregisterFunctionality() {
    FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_VERBOSE,
            "foeGraphicsResourceDeregisterFunctionality - Starting to deregister functionality")

    foeDeregisterFunctionality(foeGraphicsResourceFunctionalityID());

    FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_VERBOSE,
            "foeGraphicsResourceDeregisterFunctionality - Completed deregistering functionality")
}