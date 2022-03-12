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

#include <foe/graphics/resource/material_loader.hpp>

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/image_pool.hpp>
#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/graphics/vk/shader.hpp>
#include <vk_error_code.hpp>
#include <vk_struct_cleanup.h>
#include <vulkan/vulkan.h>

#include "error_code.hpp"
#include "log.hpp"
#include "worst_subresource_fn.hpp"

#include <type_traits>

foeMaterialCreateInfo::~foeMaterialCreateInfo() {
    if (hasColourBlendSCI)
        cleanup_VkPipelineColorBlendStateCreateInfo(&colourBlendSCI);
    if (hasDepthStencilSCI)
        cleanup_VkPipelineDepthStencilStateCreateInfo(&depthStencilSCI);
    if (hasRasterizationSCI)
        cleanup_VkPipelineRasterizationStateCreateInfo(&rasterizationSCI);
}

void foeDestroyMaterialCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeMaterialCreateInfo *)pCreateInfo;
    pCI->~foeMaterialCreateInfo();
}

auto foeMaterialLoader::initialize(foeShaderPool *pShaderPool, foeImagePool *pImagePool)
    -> std::error_code {
    if (pShaderPool == nullptr || pImagePool == nullptr)
        return FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED;

    // External
    mShaderPool = pShaderPool;
    mImagePool = pImagePool;

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

void foeMaterialLoader::deinitialize() {
    // External
    mImagePool = nullptr;
    mShaderPool = nullptr;
}

bool foeMaterialLoader::initialized() const noexcept { return mShaderPool != nullptr; }

auto foeMaterialLoader::initializeGraphics(foeGfxSession gfxSession) -> std::error_code {
    if (!initialized())
        return FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_NOT_INITIALIZED;

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

    return vkRes;
}

void foeMaterialLoader::deinitializeGraphics() {
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

void foeMaterialLoader::gfxMaintenance() {
    // Process Delayed Data Destruction
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }

    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);

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
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Process Load Requests
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        // Check to see if what we need has been loaded yet
        auto subResLoadState = getWorstSubResourceState(it.data.fragmentShader, it.data.image);

        if (subResLoadState == foeResourceLoadState::Loaded) {
            { // Using the sub-resources that are loaded, and definition data, create the resource
                foeGfxShader fragShader =
                    (it.data.fragmentShader != FOE_NULL_HANDLE)
                        ? ((foeShader const *)foeResourceGetData(it.data.fragmentShader))->shader
                        : FOE_NULL_HANDLE;

                auto const *pMaterialCI =
                    (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(it.createInfo);

                it.data.pGfxFragDescriptor = mGfxFragmentDescriptorPool->get(
                    (pMaterialCI->hasRasterizationSCI) ? &pMaterialCI->rasterizationSCI : nullptr,
                    (pMaterialCI->hasDepthStencilSCI) ? &pMaterialCI->depthStencilSCI : nullptr,
                    (pMaterialCI->hasColourBlendSCI) ? &pMaterialCI->colourBlendSCI : nullptr,
                    fragShader);
            }

            auto errC = createDescriptorSet(&it.data);
            if (errC)
                goto DESCRIPTOR_CREATE_FAILED;

            // Everything's ready, load the resource
            auto moveFn = [](void *pSrc, void *pDst) {
                auto *pSrcData = (foeMaterial *)pSrc;
                new (pDst) foeMaterial(std::move(*pSrcData));
            };

            it.pPostLoadFn(it.resource, {}, &it.data, moveFn, it.createInfo, this,
                           foeMaterialLoader::unloadResource);
        } else if (subResLoadState == foeResourceLoadState::Failed) {
        DESCRIPTOR_CREATE_FAILED:
            // One of them failed to load, we're not proceeding with this resource
            it.pPostLoadFn(
                it.resource,
                foeToErrorCode(FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD),
                nullptr, nullptr, nullptr, nullptr, nullptr);

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
            // All items are at least 'loading', so just re-queue
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
                             PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeMaterialLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeMaterialLoader::load(foeResource resource,
                             foeResourceCreateInfo createInfo,
                             PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        pPostLoadFn(resource, foeToErrorCode(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                    nullptr, nullptr, createInfo, nullptr, nullptr);
        return;
    }

    auto const *pMaterialCI =
        (foeMaterialCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeMaterial data{};

    // Fragment Shader
    if (pMaterialCI->fragmentShader != FOE_INVALID_ID) {
        data.fragmentShader = mShaderPool->findOrAdd(pMaterialCI->fragmentShader);

        foeResourceIncrementRefCount(data.fragmentShader);
        foeResourceIncrementUseCount(data.fragmentShader);
        foeResourceLoad(data.fragmentShader, false);
    }

    // Image
    if (pMaterialCI->image != FOE_INVALID_ID) {
        data.image = mImagePool->findOrAdd(pMaterialCI->image);

        foeResourceIncrementRefCount(data.image);
        foeResourceIncrementUseCount(data.image);
        foeResourceLoad(data.image, false);
    }

    // Send to the loading queue to await results
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .resource = resource,
        .createInfo = createInfo,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

std::error_code foeMaterialLoader::createDescriptorSet(foeMaterial *pMaterialData) {
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

    VkResult vkRes = vkAllocateDescriptorSets(vkDevice, &setAI, &set);
    if (vkRes != VK_SUCCESS) {
        return vkRes;
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

    return {};
}

void foeMaterialLoader::unloadResource(void *pContext,
                                       foeResource resource,
                                       uint32_t resourceIteration,
                                       PFN_foeResourceUnloadCall *pUnloadCallFn,
                                       bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeMaterialLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeMaterial *)pSrc;
            auto *pDstData = (foeMaterial *)pDst;

            *pDstData = std::move(*pSrcData);
            pSrcData->~foeMaterial();
        };

        foeMaterial data{};

        if (pUnloadCallFn(resource, resourceIteration, &data, moveFn)) {
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
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(std::move(data));
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