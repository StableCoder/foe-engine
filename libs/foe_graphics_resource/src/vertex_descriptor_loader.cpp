// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "vertex_descriptor_loader.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>

#include "log.hpp"
#include "result.h"

#include <array>
#include <cassert>
#include <cstdlib>

foeResultSet foeVertexDescriptorLoader::initialize(foeResourcePool resourcePool) {
    if (resourcePool == FOE_NULL_HANDLE) {
        return to_foeResult(
            FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_LOADER_INITIALIZATION_FAILED);
    }

    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);

    mResourcePool = resourcePool;

    if (result.value != FOE_SUCCESS) {
        deinitialize();
    }

    return result;
}

void foeVertexDescriptorLoader::deinitialize() {
    // External
    mResourcePool = nullptr;
}

bool foeVertexDescriptorLoader::initialized() const noexcept {
    return mResourcePool != FOE_NULL_HANDLE;
}

foeResultSet foeVertexDescriptorLoader::initializeGraphics(foeGfxSession gfxSession) {
    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeVertexDescriptorLoader::deinitializeGraphics() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork =
            foeResourcePoolUnloadType(mResourcePool,
                                      FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) > 0;

        gfxMaintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();
    } while (upcomingWork);
}

namespace {

bool processResourceReplacement(foeResource *pResource) {
    bool replaced = false;

    if (*pResource == FOE_NULL_HANDLE)
        return false;

    while (foeResourceGetType(*pResource) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED) {
        foeResource replacement = foeResourceGetReplacement(*pResource);

        foeResourceIncrementUseCount(replacement);

        foeResourceDecrementUseCount(*pResource);
        foeResourceDecrementRefCount(*pResource);

        *pResource = replacement;
        replaced = true;
    }

    return replaced;
}

foeResourceStateFlags processResourceLoadState(foeResource *pResource,
                                               foeResourceType resourceType,
                                               foeResourceStateFlags overallState) {
    if (*pResource == FOE_NULL_HANDLE)
        return overallState;

    foeResourceStateFlags resourceState;

    do {
        resourceState = foeResourceGetState(*pResource);
    } while (processResourceReplacement(pResource));

    if (foeResourceType type = foeResourceGetType(*pResource);
        type != resourceType && type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
        type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
        !foeResourceHasType(*pResource, resourceType)) {
        overallState |= FOE_RESOURCE_STATE_FAILED_BIT;
    }

    if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
        overallState |= FOE_RESOURCE_STATE_FAILED_BIT;
    } else if ((resourceState & FOE_RESOURCE_STATE_LOADED_BIT) == 0) {
        // This resource is not loaded, therefore remove the overall LOADED flag
        overallState &= ~FOE_RESOURCE_STATE_LOADED_BIT;

        if ((resourceState & FOE_RESOURCE_STATE_LOADING_BIT) == 0) {
            // Resource is not LOADED and not LOADING, request load
            foeResourceLoadData(*pResource);
            overallState |= FOE_RESOURCE_STATE_LOADING_BIT;
        }
    }
    if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT)
        overallState |= FOE_RESOURCE_STATE_LOADING_BIT;

    return overallState;
}

} // namespace

void foeVertexDescriptorLoader::gfxMaintenance() {
    // Unload Requests
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.unloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Load Requests
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        foeResourceStateFlags resourceState = FOE_RESOURCE_STATE_LOADED_BIT;

        resourceState = processResourceLoadState(
            &it.data.vertexShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, resourceState);
        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT)
            goto PROCESS_RESOURCE;

        resourceState =
            processResourceLoadState(&it.data.tessellationControlShader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, resourceState);
        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT)
            goto PROCESS_RESOURCE;

        resourceState =
            processResourceLoadState(&it.data.tessellationEvaluationShader,
                                     FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, resourceState);
        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT)
            goto PROCESS_RESOURCE;

        resourceState = processResourceLoadState(
            &it.data.geometryShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, resourceState);
        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT)
            goto PROCESS_RESOURCE;

    PROCESS_RESOURCE:
        assert(resourceState & (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                                FOE_RESOURCE_STATE_LOADED_BIT));

        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
            // At least one of the items failed to load
            if (it.data.vertexShader != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(it.data.vertexShader);
                foeResourceDecrementRefCount(it.data.vertexShader);
            }
            if (it.data.tessellationControlShader != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(it.data.tessellationControlShader);
                foeResourceDecrementRefCount(it.data.tessellationControlShader);
            }
            if (it.data.tessellationEvaluationShader != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(it.data.tessellationEvaluationShader);
                foeResourceDecrementRefCount(it.data.tessellationEvaluationShader);
            }
            if (it.data.geometryShader != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(it.data.geometryShader);
                foeResourceDecrementRefCount(it.data.geometryShader);
            }

            cleanup_foeVertexDescriptor(&it.data);

            it.postLoadFn(
                it.resource,
                to_foeResult(
                    FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD),
                nullptr, nullptr, nullptr, nullptr);
            foeResourceCreateInfoDecrementRefCount(it.createInfo);
        } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
            if (it.data.vertexShader != FOE_NULL_HANDLE) {
                it.data.vertexDescriptor.mVertex =
                    ((foeShader const *)foeResourceGetData(it.data.vertexShader))->shader;
            }
            if (it.data.tessellationControlShader != FOE_NULL_HANDLE) {
                it.data.vertexDescriptor.mTessellationControl =
                    ((foeShader const *)foeResourceGetData(it.data.tessellationControlShader))
                        ->shader;
            }
            if (it.data.tessellationEvaluationShader != FOE_NULL_HANDLE) {
                it.data.vertexDescriptor.mTessellationEvaluation =
                    ((foeShader const *)foeResourceGetData(it.data.tessellationEvaluationShader))
                        ->shader;
            }
            if (it.data.geometryShader != FOE_NULL_HANDLE) {
                it.data.vertexDescriptor.mGeometry =
                    ((foeShader const *)foeResourceGetData(it.data.geometryShader))->shader;
            }

            auto const *pCreateInfo =
                (foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(it.createInfo);

            it.data.vertexDescriptor.mVertexInputSCI = pCreateInfo->vertexInputSCI;

            it.data.vertexDescriptor.vertexInputBindingCount = pCreateInfo->inputBindingCount;
            it.data.vertexDescriptor.pVertexInputBindings =
                (VkVertexInputBindingDescription *)malloc(
                    it.data.vertexDescriptor.vertexInputBindingCount *
                    sizeof(VkVertexInputBindingDescription));
            memcpy(it.data.vertexDescriptor.pVertexInputBindings, pCreateInfo->pInputBindings,
                   it.data.vertexDescriptor.vertexInputBindingCount *
                       sizeof(VkVertexInputBindingDescription));

            it.data.vertexDescriptor.vertexInputAttributeCount = pCreateInfo->inputAttributeCount;
            it.data.vertexDescriptor.pVertexInputAttributes =
                (VkVertexInputAttributeDescription *)malloc(
                    it.data.vertexDescriptor.vertexInputAttributeCount *
                    sizeof(VkVertexInputAttributeDescription));
            memcpy(it.data.vertexDescriptor.pVertexInputAttributes, pCreateInfo->pInputAttributes,
                   it.data.vertexDescriptor.vertexInputAttributeCount *
                       sizeof(VkVertexInputAttributeDescription));

            it.data.vertexDescriptor.mInputAssemblySCI = pCreateInfo->inputAssemblySCI;
            it.data.vertexDescriptor.mTessellationSCI = pCreateInfo->tessellationSCI;

            // Everything looks good, lock the resource and update it
            auto moveFn = [](void *pSrc, void *pDst) {
                auto *pSrcData = (foeVertexDescriptor *)pSrc;
                new (pDst) foeVertexDescriptor{*pSrcData};
            };

            if (foeResourceGetType(it.resource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
                // Need to replace the placeholder with the actual resource
                foeResource newResource = foeResourcePoolLoadedReplace(
                    mResourcePool, foeResourceGetID(it.resource),
                    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
                    sizeof(foeVertexDescriptor), &it.data, moveFn, this,
                    foeVertexDescriptorLoader::unloadResource);

                if (newResource == FOE_NULL_HANDLE)
                    // @TODO - Handle failure
                    std::abort();

                foeResourceDecrementRefCount(it.resource);
                foeResourceDecrementRefCount(newResource);
            } else {
                it.postLoadFn(it.resource, {}, &it.data, moveFn, this,
                              foeVertexDescriptorLoader::unloadResource);
            }

            foeResourceCreateInfoDecrementRefCount(it.createInfo);
        } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
            // Sub-items are still at least loading
            stillLoading.emplace_back(it);
        }
    }

    // If there are items still loading, requeue them
    if (!stillLoading.empty()) {
        mLoadSync.lock();

        mLoadRequests.reserve(mLoadRequests.size() + stillLoading.size());
        for (auto &it : stillLoading) {
            mLoadRequests.emplace_back(it);
        }

        mLoadSync.unlock();
    }
}

bool foeVertexDescriptorLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR_CREATE_INFO;
}

void foeVertexDescriptorLoader::load(void *pLoader,
                                     foeResource resource,
                                     foeResourceCreateInfo createInfo,
                                     PFN_foeResourcePostLoad postLoadFn) {
    reinterpret_cast<foeVertexDescriptorLoader *>(pLoader)->load(resource, createInfo, postLoadFn);
}

void foeVertexDescriptorLoader::load(foeResource resource,
                                     foeResourceCreateInfo createInfo,
                                     PFN_foeResourcePostLoad postLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeVertexDescriptorLoader - Cannot load {} as given CreateInfo is "
                "incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)),
                foeResourceCreateInfoGetType(createInfo));

        postLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                   nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    } else if (foeResourceType type = foeResourceGetType(resource);
               type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR &&
               type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeVertexDescriptorLoader - Cannot load {} as it is an incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)), foeResourceGetType(resource));

        postLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_RESOURCE_TYPE),
                   nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    }

    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
    foeVertexDescriptorCreateInfo const *pCI =
        (foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
    foeVertexDescriptor data{
        .rType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR,
    };

    // Find all required sub-resources, and make sure they are compatible types
    if (pCI->vertexShader != FOE_INVALID_ID) {
        while (data.vertexShader == FOE_NULL_HANDLE) {
            data.vertexShader = foeResourcePoolFind(mResourcePool, pCI->vertexShader);

            if (data.vertexShader == FOE_NULL_HANDLE)
                data.vertexShader = foeResourcePoolAdd(mResourcePool, pCI->vertexShader);
        }

        if (foeResourceType type = foeResourceGetType(data.vertexShader);
            type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER &&
            type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
            type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
            !foeResourceHasType(data.vertexShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER)) {
            result = to_foeResult(
                FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_INCOMPATIBLE);
            goto LOAD_FAILED;
        }
    }
    if (pCI->tessellationControlShader != FOE_INVALID_ID) {
        while (data.tessellationControlShader == FOE_NULL_HANDLE) {
            data.tessellationControlShader =
                foeResourcePoolFind(mResourcePool, pCI->tessellationControlShader);

            if (data.tessellationControlShader == FOE_NULL_HANDLE)
                data.tessellationControlShader =
                    foeResourcePoolAdd(mResourcePool, pCI->tessellationControlShader);
        }

        if (foeResourceType type = foeResourceGetType(data.tessellationControlShader);
            type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER &&
            type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
            type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
            !foeResourceHasType(data.tessellationControlShader,
                                FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER)) {
            result = to_foeResult(
                FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_INCOMPATIBLE);
            goto LOAD_FAILED;
        }
    }
    if (pCI->tessellationEvaluationShader != FOE_INVALID_ID) {
        while (data.tessellationEvaluationShader == FOE_NULL_HANDLE) {
            data.tessellationEvaluationShader =
                foeResourcePoolFind(mResourcePool, pCI->tessellationEvaluationShader);

            if (data.tessellationEvaluationShader == FOE_NULL_HANDLE)
                data.tessellationEvaluationShader =
                    foeResourcePoolAdd(mResourcePool, pCI->tessellationEvaluationShader);
        }

        if (foeResourceType type = foeResourceGetType(data.tessellationEvaluationShader);
            type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER &&
            type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
            type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
            !foeResourceHasType(data.tessellationEvaluationShader,
                                FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER)) {
            result = to_foeResult(
                FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_INCOMPATIBLE);
            goto LOAD_FAILED;
        }
    }
    if (pCI->geometryShader != FOE_INVALID_ID) {
        while (data.geometryShader == FOE_NULL_HANDLE) {
            data.geometryShader = foeResourcePoolFind(mResourcePool, pCI->geometryShader);

            if (data.geometryShader == FOE_NULL_HANDLE)
                data.geometryShader = foeResourcePoolAdd(mResourcePool, pCI->geometryShader);
        }

        if (foeResourceType type = foeResourceGetType(data.geometryShader);
            type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER &&
            type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
            type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
            !foeResourceHasType(data.geometryShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER)) {
            result = to_foeResult(
                FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_INCOMPATIBLE);
            goto LOAD_FAILED;
        }
    }

    // If here, we have all requested resources, so increment their use, then make sure they are
    // loaded or loading. We only want to attempt to load *any* of the sub-resources if we can find
    // them all as compatible types
    if (data.vertexShader != FOE_NULL_HANDLE) {
        foeResourceIncrementUseCount(data.vertexShader);
        if ((foeResourceGetState(data.vertexShader) &
             (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT)) == 0)
            foeResourceLoadData(data.vertexShader);
    }
    if (data.tessellationControlShader != FOE_NULL_HANDLE) {
        foeResourceIncrementUseCount(data.tessellationControlShader);
        if ((foeResourceGetState(data.tessellationControlShader) &
             (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT)) == 0)
            foeResourceLoadData(data.tessellationControlShader);
    }
    if (data.tessellationEvaluationShader != FOE_NULL_HANDLE) {
        foeResourceIncrementUseCount(data.tessellationEvaluationShader);
        if ((foeResourceGetState(data.tessellationEvaluationShader) &
             (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT)) == 0)
            foeResourceLoadData(data.tessellationEvaluationShader);
    }
    if (data.geometryShader != FOE_NULL_HANDLE) {
        foeResourceIncrementUseCount(data.geometryShader);
        if ((foeResourceGetState(data.geometryShader) &
             (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT)) == 0)
            foeResourceLoadData(data.geometryShader);
    }

    // Send to the loading queue
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .resource = resource,
        .createInfo = createInfo,
        .postLoadFn = postLoadFn,
        .data = data,
    });
    mLoadSync.unlock();
    return;

LOAD_FAILED:
    if (data.vertexShader != FOE_NULL_HANDLE)
        foeResourceDecrementRefCount(data.vertexShader);
    if (data.tessellationControlShader != FOE_NULL_HANDLE)
        foeResourceDecrementRefCount(data.tessellationControlShader);
    if (data.tessellationEvaluationShader != FOE_NULL_HANDLE)
        foeResourceDecrementRefCount(data.tessellationEvaluationShader);
    if (data.geometryShader != FOE_NULL_HANDLE)
        foeResourceDecrementRefCount(data.geometryShader);

    // Call the resource post-load function with the error result code
    postLoadFn(resource, result, nullptr, nullptr, nullptr, nullptr);
    foeResourceCreateInfoDecrementRefCount(createInfo);
}

void foeVertexDescriptorLoader::unloadResource(void *pContext,
                                               foeResource resource,
                                               uint32_t resourceIteration,
                                               PFN_foeResourceUnloadCall unloadCallFn,
                                               bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeVertexDescriptorLoader *>(pContext);

    if (immediateUnload) {
        auto unloadDataFn = [](void *pLoaderContext, void *pResourceRawData) {
            foeVertexDescriptor *pLoaderData = (foeVertexDescriptor *)pLoaderContext;
            foeVertexDescriptor *pResourceData = (foeVertexDescriptor *)pResourceRawData;

            *pLoaderData = *pResourceData;
        };

        foeVertexDescriptor data;

        if (!unloadCallFn(resource, resourceIteration, &data, unloadDataFn)) {
            // If it failed, it's probably due to the resource iteration being different than
            // desired, so it didn't happen.
            return;
        }

        cleanup_foeVertexDescriptor(&data);

        // Decrement the ref/use count of any sub-resources
        if (data.vertexShader != FOE_NULL_HANDLE) {
            foeResourceDecrementUseCount(data.vertexShader);
            foeResourceDecrementRefCount(data.vertexShader);
        }
        if (data.tessellationControlShader != FOE_NULL_HANDLE) {
            foeResourceDecrementUseCount(data.tessellationControlShader);
            foeResourceDecrementRefCount(data.tessellationControlShader);
        }
        if (data.tessellationEvaluationShader != FOE_NULL_HANDLE) {
            foeResourceDecrementUseCount(data.tessellationEvaluationShader);
            foeResourceDecrementRefCount(data.tessellationEvaluationShader);
        }
        if (data.geometryShader != FOE_NULL_HANDLE) {
            foeResourceDecrementUseCount(data.geometryShader);
            foeResourceDecrementRefCount(data.geometryShader);
        }
    } else {
        foeResourceIncrementRefCount(resource);
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .unloadCallFn = unloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}