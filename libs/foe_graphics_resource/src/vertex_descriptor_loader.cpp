// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/vertex_descriptor_loader.hpp>

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/resource/vertex_descriptor_create_info.h>

#include "log.hpp"
#include "result.h"
#include "worst_resource_state.hpp"

#include <array>
#include <stdlib.h>

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
    // Unload all resources this loader loaded
    // @todo This should probably be under a gfxDeinitialize instead
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

    // External
    mResourcePool = nullptr;
}

bool foeVertexDescriptorLoader::initialized() const noexcept {
    return mResourcePool != FOE_NULL_HANDLE;
}

void foeVertexDescriptorLoader::gfxMaintenance() {
    // Unload Requests
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Load Requests
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        std::array<foeResource, 4> subResources = {
            it.data.vertexShader,
            it.data.tessellationControlShader,
            it.data.tessellationEvaluationShader,
            it.data.geometryShader,
        };

        auto subResLoadState = worstResourceLoadState(subResources.size(), subResources.data());

        if (subResLoadState == FOE_RESOURCE_LOAD_STATE_LOADED) {
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

            it.pPostLoadFn(it.resource, {}, &it.data, moveFn, it.createInfo, this,
                           foeVertexDescriptorLoader::unloadResource);

        } else if (subResLoadState == FOE_RESOURCE_LOAD_STATE_FAILED) {
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

            it.pPostLoadFn(
                it.resource,
                to_foeResult(
                    FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD),
                nullptr, nullptr, nullptr, nullptr, nullptr);
        } else {
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
                                     PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeVertexDescriptorLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeVertexDescriptorLoader::load(foeResource resource,
                                     foeResourceCreateInfo createInfo,
                                     PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo) ||
        foeResourceGetType(resource) != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) {
        foeGraphicsResourceResult result;
        if (foeResourceGetType(resource) !=
            FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_VERTEX_DESCRIPTOR) {
            result = FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_RESOURCE_TYPE;
            FOE_LOG(foeGraphicsResource, Error,
                    "foeVertexDescriptorLoader - Cannot load {} as it is an incompatible type: {}",
                    foeIdToString(foeResourceGetID(resource)), foeResourceGetType(resource));
        } else {
            result = FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO;
            FOE_LOG(foeGraphicsResource, Error,
                    "foeVertexDescriptorLoader - Cannot load {} as given CreateInfo is "
                    "incompatible type: {}",
                    foeIdToString(foeResourceGetID(resource)),
                    foeResourceCreateInfoGetType(createInfo));
        }

        pPostLoadFn(resource, to_foeResult(result), nullptr, nullptr, nullptr, nullptr, nullptr);
        return;
    }

    auto const *pCI =
        (foeVertexDescriptorCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeVertexDescriptor data{};

    // Find all of the subresources
    if (pCI->vertexShader != FOE_INVALID_ID) {
        while (data.vertexShader == FOE_NULL_HANDLE) {
            data.vertexShader = foeResourcePoolFind(mResourcePool, pCI->vertexShader);

            if (data.vertexShader == FOE_NULL_HANDLE)
                data.vertexShader = foeResourcePoolAdd(mResourcePool, pCI->vertexShader,
                                                       FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
                                                       sizeof(foeShader));
        }

        foeResourceIncrementRefCount(data.vertexShader);
        foeResourceIncrementUseCount(data.vertexShader);

        if (foeResourceGetState(data.vertexShader) != FOE_RESOURCE_LOAD_STATE_LOADED &&
            !foeResourceGetIsLoading(data.vertexShader))
            foeResourceLoad(data.vertexShader, false);
    }
    if (pCI->tessellationControlShader != FOE_INVALID_ID) {
        while (data.tessellationControlShader == FOE_NULL_HANDLE) {
            data.tessellationControlShader =
                foeResourcePoolFind(mResourcePool, pCI->tessellationControlShader);

            if (data.tessellationControlShader == FOE_NULL_HANDLE)
                data.tessellationControlShader = foeResourcePoolAdd(
                    mResourcePool, pCI->tessellationControlShader,
                    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, sizeof(foeShader));
        }

        foeResourceIncrementRefCount(data.tessellationControlShader);
        foeResourceIncrementUseCount(data.tessellationControlShader);

        if (foeResourceGetState(data.tessellationControlShader) != FOE_RESOURCE_LOAD_STATE_LOADED &&
            !foeResourceGetIsLoading(data.tessellationControlShader))
            foeResourceLoad(data.tessellationControlShader, false);
    }
    if (pCI->tessellationEvaluationShader != FOE_INVALID_ID) {
        while (data.tessellationEvaluationShader == FOE_NULL_HANDLE) {
            data.tessellationEvaluationShader =
                foeResourcePoolFind(mResourcePool, pCI->tessellationEvaluationShader);

            if (data.tessellationEvaluationShader == FOE_NULL_HANDLE)
                data.tessellationEvaluationShader = foeResourcePoolAdd(
                    mResourcePool, pCI->tessellationEvaluationShader,
                    FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, sizeof(foeShader));
        }

        foeResourceIncrementRefCount(data.tessellationEvaluationShader);
        foeResourceIncrementUseCount(data.tessellationEvaluationShader);

        if (foeResourceGetState(data.tessellationEvaluationShader) !=
                FOE_RESOURCE_LOAD_STATE_LOADED &&
            !foeResourceGetIsLoading(data.tessellationEvaluationShader))
            foeResourceLoad(data.tessellationEvaluationShader, false);
    }
    if (pCI->geometryShader != FOE_INVALID_ID) {
        while (data.geometryShader == FOE_NULL_HANDLE) {
            data.geometryShader = foeResourcePoolFind(mResourcePool, pCI->geometryShader);

            if (data.geometryShader == FOE_NULL_HANDLE)
                data.geometryShader = foeResourcePoolAdd(
                    mResourcePool, pCI->geometryShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
                    sizeof(foeShader));
        }

        foeResourceIncrementRefCount(data.geometryShader);
        foeResourceIncrementUseCount(data.geometryShader);

        if (foeResourceGetState(data.geometryShader) != FOE_RESOURCE_LOAD_STATE_LOADED &&
            !foeResourceGetIsLoading(data.geometryShader))
            foeResourceLoad(data.geometryShader, false);
    }

    // Send to the loading queue
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .resource = resource,
        .createInfo = createInfo,
        .pPostLoadFn = pPostLoadFn,
        .data = data,
    });
    mLoadSync.unlock();
}

void foeVertexDescriptorLoader::unloadResource(void *pContext,
                                               foeResource resource,
                                               uint32_t resourceIteration,
                                               PFN_foeResourceUnloadCall *pUnloadCallFn,
                                               bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeVertexDescriptorLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeVertexDescriptor *)pSrc;
            auto *pDstData = (foeVertexDescriptor *)pDst;

            *pDstData = *pSrcData;
        };

        foeVertexDescriptor data;

        if (!pUnloadCallFn(resource, resourceIteration, &data, moveFn)) {
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
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}