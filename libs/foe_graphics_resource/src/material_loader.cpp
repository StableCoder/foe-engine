/*
    Copyright (C) 2021 George Cave.

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
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/graphics/vk/shader.hpp>
#include <vk_error_code.hpp>
#include <vk_struct_cleanup.hpp>
#include <vulkan/vulkan.h>

#include "error_code.hpp"
#include "log.hpp"

#include <type_traits>

namespace {

template <typename SubResource, typename... SubResources>
foeResourceState getWorstSubResourceState(SubResource *pSubResource,
                                          SubResources *...pSubResources) {
    if (pSubResource != nullptr) {
        auto state = pSubResource->getState();
        if (state != foeResourceState::Loaded) {
            return state;
        }
    }

    if constexpr (sizeof...(SubResources) != 0) {
        // Not the last provided one, keep going
        return getWorstSubResourceState(pSubResources...);
    } else {
        // End of the line, return that they're all at least in the good 'loaded' state
        return foeResourceState::Loaded;
    }
}

} // namespace

foeMaterialCreateInfo::~foeMaterialCreateInfo() {
    if (hasColourBlendSCI)
        vk_struct_cleanup(&colourBlendSCI);
    if (hasDepthStencilSCI)
        vk_struct_cleanup(&depthStencilSCI);
    if (hasRasterizationSCI)
        vk_struct_cleanup(&rasterizationSCI);
}

std::error_code foeMaterialLoader::initialize(foeShaderPool *pShaderPool,
                                              foeImagePool *pImagePool,
                                              foeGfxSession session) {
    if (pShaderPool == nullptr || pImagePool == nullptr) {
        return FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED;
    }
    if (session == FOE_NULL_HANDLE) {
        return FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_LOADER_INITIALIZATION_FAILED;
    }

    std::error_code errC{FOE_GRAPHICS_RESOURCE_SUCCESS};

    mShaderPool = pShaderPool;
    mImagePool = pImagePool;
    mGfxSession = session;

    mGfxFragmentDescriptorPool = foeGfxVkGetFragmentDescriptorPool(session);

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

    VkDevice device = foeGfxVkGetDevice(session);
    VkResult vkRes = vkCreateDescriptorPool(device, &poolCI, nullptr, &mDescriptorPool);
    if (vkRes != VK_SUCCESS) {
        errC = vkRes;
        goto INITIALIZATION_FAILED;
    }

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeMaterialLoader::deinitialize() {
    if (mDescriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(foeGfxVkGetDevice(mGfxSession), mDescriptorPool, nullptr);

    mDescriptorPool = VK_NULL_HANDLE;

    mGfxFragmentDescriptorPool = nullptr;

    mGfxSession = FOE_NULL_HANDLE;
    mImagePool = nullptr;
    mShaderPool = nullptr;
}

bool foeMaterialLoader::initialized() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

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
        unloadResource(this, it.pMaterial, it.iteration, true);
        it.pMaterial->decrementRefCount();
    }

    // Process Load Requests
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        // Check to see if what we need has been loaded yet
        auto subResLoadState = getWorstSubResourceState(it.data.pFragmentShader, it.data.pImage);

        if (subResLoadState == foeResourceState::Loaded) {
            { // Using the sub-resources that are loaded, and definition data, create the resource
                foeGfxShader fragShader = (it.data.pFragmentShader != nullptr)
                                              ? it.data.pFragmentShader->data.shader
                                              : FOE_NULL_HANDLE;

                auto *pMaterialCI =
                    reinterpret_cast<foeMaterialCreateInfo *>(it.data.pCreateInfo.get());

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
            it.pMaterial->modifySync.lock();

            if (it.pMaterial->data.pUnloadFn != nullptr) {
                it.pMaterial->data.pUnloadFn(it.pMaterial->data.pUnloadContext, it.pMaterial,
                                             it.pMaterial->iteration, true);
            }

            ++it.pMaterial->iteration;
            it.pMaterial->data = std::move(it.data);
            it.pPostLoadFn(it.pMaterial, {});

            it.pMaterial->modifySync.unlock();
        } else if (subResLoadState == foeResourceState::Failed) {
        DESCRIPTOR_CREATE_FAILED:
            // One of them failed to load, we're not proceeding with this resource
            it.pPostLoadFn(it.pMaterial,
                           FOE_GRAPHICS_RESOURCE_ERROR_MATERIAL_SUBRESOURCE_FAILED_TO_LOAD);

            // Unload the data we did get
            if (it.data.pFragmentShader != nullptr)
                it.data.pFragmentShader->decrementRefCount();
            if (it.data.pImage != nullptr)
                it.data.pImage->decrementRefCount();
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

bool foeMaterialLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeMaterialCreateInfo *>(pCreateInfo) != nullptr;
}

void foeMaterialLoader::load(void *pResource,
                             std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                             void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pMaterial = reinterpret_cast<foeMaterial *>(pResource);
    auto *pMaterialCI = reinterpret_cast<foeMaterialCreateInfo *>(pCreateInfo.get());

    foeMaterial::Data materialData{
        .pCreateInfo = pCreateInfo,
    };

    // Fragment Shader
    if (pMaterialCI->fragmentShader != FOE_INVALID_ID) {
        materialData.pFragmentShader = mShaderPool->findOrAdd(pMaterialCI->fragmentShader);

        materialData.pFragmentShader->incrementRefCount();
        materialData.pFragmentShader->incrementUseCount();
        materialData.pFragmentShader->loadResource(false);
    }

    // Image
    if (pMaterialCI->image != FOE_INVALID_ID) {
        materialData.pImage = mImagePool->findOrAdd(pMaterialCI->image);

        materialData.pImage->incrementRefCount();
        materialData.pImage->incrementUseCount();
        materialData.pImage->loadResource(false);
    }

    // Send to the loading queue to await results
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .pMaterial = pMaterial,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(materialData),
    });
    mLoadSync.unlock();
}

std::error_code foeMaterialLoader::createDescriptorSet(foeMaterial::Data *pMaterialData) {
    // If there's no elements to create a descriptor set with, just return
    if (pMaterialData->pImage == nullptr) {
        return {};
    }

    VkDescriptorSet set;
    VkDevice vkDevice = foeGfxVkGetDevice(mGfxSession);

    auto descriptorSetLayout =
        foeGfxVkGetShaderDescriptorSetLayout(pMaterialData->pFragmentShader->data.shader);

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

    VkDescriptorImageInfo imageInfo{
        .sampler = pMaterialData->pImage->data.sampler,
        .imageView = pMaterialData->pImage->data.view,
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
                                       void *pResource,
                                       uint32_t resourceIteration,
                                       bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeMaterialLoader *>(pContext);
    auto *pMaterial = reinterpret_cast<foeMaterial *>(pResource);

    if (immediateUnload) {
        pMaterial->modifySync.lock();

        if (pMaterial->iteration == resourceIteration) {
            auto data = std::move(pMaterial->data);

            pMaterial->data = {};
            pMaterial->state = foeResourceState::Unloaded;
            ++pMaterial->iteration;

            // Decrement the references of any sub-resources
            if (data.pFragmentShader != nullptr) {
                data.pFragmentShader->decrementUseCount();
                data.pFragmentShader->decrementRefCount();
            }
            if (data.pImage != nullptr) {
                data.pImage->decrementUseCount();
                data.pImage->decrementRefCount();
            }

            // Queue for delayed destruction
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(std::move(data));
        }

        pMaterial->modifySync.unlock();
    } else {
        pMaterial->incrementRefCount();
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pMaterial = pMaterial,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadSync.unlock();
    }
}