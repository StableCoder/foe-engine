// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "material_loader.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/material_create_info.h>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/fragment_descriptor_pool.h>
#include <foe/graphics/vk/session.h>
#include <foe/graphics/vk/shader.h>

#include "log.hpp"
#include "result.h"
#include "vk_result.h"

#include <array>
#include <cassert>
#include <type_traits>

foeResultSet foeMaterialLoader::initialize(foeResourcePool resourcePool) {
    if (resourcePool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED);

    // External
    mResourcePool = resourcePool;

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeMaterialLoader::deinitialize() {
    // External
    mResourcePool = FOE_NULL_HANDLE;
}

bool foeMaterialLoader::initialized() const noexcept { return mResourcePool != FOE_NULL_HANDLE; }

foeResultSet foeMaterialLoader::initializeGraphics(foeGfxSession gfxSession) {
    if (!initialized())
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_NOT_INITIALIZED);

    // External
    mGfxSession = gfxSession;
    mGfxFragmentDescriptorPool = foeGfxVkGetFragmentDescriptorPool(gfxSession);

    // Internal
    VkDescriptorPoolSize size{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1024,
    };

    VkDescriptorPoolCreateInfo poolCI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1024,
        .poolSizeCount = 1,
        .pPoolSizes = &size,
    };

    VkDevice device = foeGfxVkGetDevice(gfxSession);
    VkResult vkRes = vkCreateDescriptorPool(device, &poolCI, nullptr, &mDescriptorPool);
    if (vkRes != VK_SUCCESS) {
        deinitialize();
    }

    return vk_to_foeResult(vkRes);
}

void foeMaterialLoader::deinitializeGraphics() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork = foeResourcePoolUnloadType(mResourcePool,
                                                 FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL) > 0;

        gfxMaintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();

        mDestroySync.lock();
        for (auto const &it : mDataDestroyLists) {
            upcomingWork |= !it.empty();
        }
        mDestroySync.unlock();
    } while (upcomingWork);

    // Internal
    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(foeGfxVkGetDevice(mGfxSession), mDescriptorPool, nullptr);
    mDescriptorPool = VK_NULL_HANDLE;

    // External
    mGfxFragmentDescriptorPool = nullptr;
    mGfxSession = FOE_NULL_HANDLE;
}

bool foeMaterialLoader::initializedGraphics() const noexcept {
    return mGfxSession != FOE_NULL_HANDLE;
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

void foeMaterialLoader::gfxMaintenance() {
    // Process Delayed Data Destruction
    mDestroySync.lock();
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }
    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);
    mDestroySync.unlock();

    auto vkDevice = foeGfxVkGetDevice(mGfxSession);

    for (auto const &it : toDestroy) {
        // Free any descriptor sets
        if (it.materialDescriptorSet != VK_NULL_HANDLE) {
            vkFreeDescriptorSets(vkDevice, mDescriptorPool, 1, &it.materialDescriptorSet);
        }
    }

    // Process Unload Requests
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.unloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Process Load Requests
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        foeResourceStateFlags resourceState = FOE_RESOURCE_STATE_LOADED_BIT;

        resourceState = processResourceLoadState(
            &it.data.fragmentShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, resourceState);
        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT)
            goto PROCESS_RESOURCE;

        resourceState = processResourceLoadState(
            &it.data.image, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE, resourceState);
        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT)
            goto PROCESS_RESOURCE;

    PROCESS_RESOURCE:
        assert(resourceState & (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT |
                                FOE_RESOURCE_STATE_LOADED_BIT));

        if (resourceState & FOE_RESOURCE_STATE_FAILED_BIT) {
            // A subresource failed to load or is the incorrect type, cannot proceed to load this
            // resource, remove references to subresources
            it.postLoadFn(
                it.resource,
                to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD),
                nullptr, nullptr, nullptr, nullptr);
            foeResourceCreateInfoDecrementRefCount(it.createInfo);

            if (it.data.fragmentShader != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(it.data.fragmentShader);
                foeResourceDecrementRefCount(it.data.fragmentShader);
            }
            if (it.data.image != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(it.data.image);
                foeResourceDecrementRefCount(it.data.image);
            }
        } else if (resourceState & FOE_RESOURCE_STATE_LOADED_BIT) {
            // Using the sub-resources that are loaded, and definition data, create the resource
            foeGfxShader fragShader =
                (it.data.fragmentShader != FOE_NULL_HANDLE)
                    ? ((foeShader const *)foeResourceGetTypeData(
                           it.data.fragmentShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER))
                          ->shader
                    : FOE_NULL_HANDLE;

            auto const *pMaterialCI =
                (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(it.createInfo);

            it.data.pGfxFragDescriptor = foeGfxVkGetFragmentDescriptor(
                mGfxFragmentDescriptorPool, pMaterialCI->pRasterizationSCI,
                pMaterialCI->pDepthStencilSCI, pMaterialCI->pColourBlendSCI, fragShader);

            VkResult vkResult = createDescriptorSet(&it.data);
            if (vkResult != VK_SUCCESS) {
                // Failed to load
                it.postLoadFn(
                    it.resource,
                    to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD),
                    nullptr, nullptr, nullptr, nullptr);
                foeResourceCreateInfoDecrementRefCount(it.createInfo);

                // Unload the data we did get
                if (it.data.fragmentShader != nullptr) {
                    foeResourceDecrementUseCount(it.data.fragmentShader);
                    foeResourceDecrementRefCount(it.data.fragmentShader);
                }
                if (it.data.image != FOE_NULL_HANDLE) {
                    foeResourceDecrementUseCount(it.data.image);
                    foeResourceDecrementRefCount(it.data.image);
                }
            } else {
                // Everything's ready, load the resource
                auto moveFn = [](void *pSrc, void *pDst) {
                    auto *pSrcData = (foeMaterial *)pSrc;
                    new (pDst) foeMaterial(std::move(*pSrcData));
                };

                if (foeResourceGetType(it.resource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
                    // Need to replace the placeholder with the actual resource
                    foeResource newResource = foeResourcePoolLoadedReplace(
                        mResourcePool, foeResourceGetID(it.resource),
                        FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL, sizeof(foeMaterial),
                        &it.data, moveFn, this, foeMaterialLoader::unloadResource);

                    if (newResource == FOE_NULL_HANDLE)
                        // @TODO - Handle failure
                        std::abort();

                    foeResourceDecrementRefCount(it.resource);
                    foeResourceDecrementRefCount(newResource);
                } else {
                    it.postLoadFn(it.resource, {}, &it.data, moveFn, this,
                                  foeMaterialLoader::unloadResource);
                }

                foeResourceCreateInfoDecrementRefCount(it.createInfo);
            }
        } else if (resourceState & FOE_RESOURCE_STATE_LOADING_BIT) {
            // Otherwise content is still LOADING, add to be reprocessed later
            stillLoading.emplace_back(std::move(it));
        }
    }

    // If there's items still loading, then requeue them
    if (!stillLoading.empty()) {
        mLoadSync.lock();

        mLoadRequests.reserve(mLoadRequests.size() + stillLoading.size());
        for (auto &it : stillLoading) {
            mLoadRequests.emplace_back(std::move(it));
        }

        mLoadSync.unlock();
    }
}

bool foeMaterialLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL_CREATE_INFO;
}

void foeMaterialLoader::load(void *pLoader,
                             foeResource resource,
                             foeResourceCreateInfo createInfo,
                             PFN_foeResourcePostLoad postLoadFn) {
    reinterpret_cast<foeMaterialLoader *>(pLoader)->load(resource, createInfo, postLoadFn);
}

void foeMaterialLoader::load(foeResource resource,
                             foeResourceCreateInfo createInfo,
                             PFN_foeResourcePostLoad postLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeMaterialLoader - Cannot load {} as given CreateInfo is incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)),
                foeResourceCreateInfoGetType(createInfo));

        postLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                   nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    } else if (foeResourceType type = foeResourceGetType(resource);
               type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL &&
               type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeMaterialLoader - Cannot load {} as it is an incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)), type);

        postLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_RESOURCE_TYPE),
                   nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    }

    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
    foeMaterialCreateInfo const *pMaterialCI =
        (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
    foeMaterial data{
        .rType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_MATERIAL,
    };

    // Find all required sub-resources, and make sure they are compatible types
    if (pMaterialCI->fragmentShader != FOE_INVALID_ID) {
        while (data.fragmentShader == FOE_NULL_HANDLE) {
            data.fragmentShader = foeResourcePoolFind(mResourcePool, pMaterialCI->fragmentShader);

            if (data.fragmentShader == FOE_NULL_HANDLE)
                data.fragmentShader =
                    foeResourcePoolAdd(mResourcePool, pMaterialCI->fragmentShader);
        }

        if (foeResourceType type = foeResourceGetType(data.fragmentShader);
            type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER &&
            type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
            type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
            !foeResourceHasType(data.fragmentShader, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER)) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_INCOMPATIBLE);
            goto LOAD_FAILED;
        }
    }

    if (pMaterialCI->image != FOE_INVALID_ID) {
        while (data.image == FOE_NULL_HANDLE) {
            data.image = foeResourcePoolFind(mResourcePool, pMaterialCI->image);

            if (data.image == FOE_NULL_HANDLE)
                data.image = foeResourcePoolAdd(mResourcePool, pMaterialCI->image);
        }

        if (foeResourceType type = foeResourceGetType(data.image);
            type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE &&
            type != FOE_RESOURCE_RESOURCE_TYPE_REPLACED &&
            type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED &&
            !foeResourceHasType(data.image, FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE)) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_INCOMPATIBLE);
            goto LOAD_FAILED;
        }
    }

    // If here, we have all requested resources, so increment their use, then make sure they are
    // loaded or loading. We only want to attempt to load *any* of the sub-resources if we can find
    // them all as compatible types
    if (data.fragmentShader != FOE_NULL_HANDLE) {
        foeResourceIncrementUseCount(data.fragmentShader);
        if ((foeResourceGetState(data.fragmentShader) &
             (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT)) == 0)
            foeResourceLoadData(data.fragmentShader);
    }
    if (data.image != FOE_NULL_HANDLE) {
        foeResourceIncrementUseCount(data.image);
        if ((foeResourceGetState(data.image) &
             (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT)) == 0)
            foeResourceLoadData(data.image);
    }

    // Send to the loading queue to await results
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .resource = resource,
        .createInfo = createInfo,
        .postLoadFn = postLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
    return;

LOAD_FAILED:
    if (data.fragmentShader != FOE_NULL_HANDLE)
        foeResourceDecrementRefCount(data.fragmentShader);
    if (data.image != FOE_NULL_HANDLE)
        foeResourceDecrementRefCount(data.image);

    // Call the resource post-load function with the error result code
    postLoadFn(resource, result, nullptr, nullptr, nullptr, nullptr);
    foeResourceCreateInfoDecrementRefCount(createInfo);
}

VkResult foeMaterialLoader::createDescriptorSet(foeMaterial *pMaterialData) {
    // If there's no elements to create a descriptor set with, just return
    if (pMaterialData->image == FOE_NULL_HANDLE) {
        return {};
    }

    auto const *pShader = (foeShader const *)foeResourceGetData(pMaterialData->fragmentShader);

    VkDescriptorSet set;
    VkDevice vkDevice = foeGfxVkGetDevice(mGfxSession);

    auto descriptorSetLayout = foeGfxVkGetShaderDescriptorSetLayout(pShader->shader);

    VkDescriptorSetAllocateInfo setAI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mDescriptorPool,
        .descriptorSetCount = 1U,
        .pSetLayouts = &descriptorSetLayout,
    };

    VkResult vkResult = vkAllocateDescriptorSets(vkDevice, &setAI, &set);
    if (vkResult != VK_SUCCESS) {
        return vkResult;
    }

    auto const *pImage = (foeImage const *)foeResourceGetData(pMaterialData->image);

    VkDescriptorImageInfo imageInfo{
        .sampler = pImage->sampler,
        .imageView = pImage->view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    VkWriteDescriptorSet writeDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
    };

    vkUpdateDescriptorSets(vkDevice, 1, &writeDescriptorSet, 0, nullptr);

    pMaterialData->materialDescriptorSet = set;

    return VK_SUCCESS;
}

void foeMaterialLoader::unloadResource(void *pContext,
                                       foeResource resource,
                                       uint32_t resourceIteration,
                                       PFN_foeResourceUnloadCall unloadCallFn,
                                       bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeMaterialLoader *>(pContext);

    if (immediateUnload) {
        auto unloadDataFn = [](void *pLoaderContext, void *pResourceRawData) {
            foeMaterial *pLoaderData = (foeMaterial *)pLoaderContext;
            foeMaterial *pResourceData = (foeMaterial *)pResourceRawData;

            *pLoaderData = *pResourceData;
        };

        foeMaterial data;

        if (unloadCallFn(resource, resourceIteration, &data, unloadDataFn)) {
            // Decrement the references of any sub-resources
            if (data.fragmentShader != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(data.fragmentShader);
                foeResourceDecrementRefCount(data.fragmentShader);
            }
            if (data.image != FOE_NULL_HANDLE) {
                foeResourceDecrementUseCount(data.image);
                foeResourceDecrementRefCount(data.image);
            }

            // Queue for delayed destruction
            pLoader->mDestroySync.lock();
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(std::move(data));
            pLoader->mDestroySync.unlock();
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