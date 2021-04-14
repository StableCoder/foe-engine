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

#include <foe/resource/material_loader.hpp>

#include <foe/ecs/id.hpp>
#include <foe/graphics/vk/fragment_descriptor_pool.hpp>
#include <foe/graphics/vk/session.hpp>
#include <foe/resource/image.hpp>
#include <foe/resource/image_pool.hpp>
#include <foe/resource/material.hpp>
#include <foe/resource/shader_pool.hpp>
#include <vk_error_code.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeMaterialLoader::~foeMaterialLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeMaterialLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeMaterialLoader::initialize(
    foeShaderLoader *pShaderLoader,
    foeShaderPool *pShaderPool,
    foeGfxVkFragmentDescriptorPool *pGfxFragmentDescriptorPool,
    foeImageLoader *pImageLoader,
    foeImagePool *pImagePool,
    foeGfxSession session,
    std::function<bool(foeId, foeResourceCreateInfoBase **)> importFunction,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mShaderLoader = pShaderLoader;
    mShaderPool = pShaderPool;
    mGfxFragmentDescriptorPool = pGfxFragmentDescriptorPool;
    mImageLoader = pImageLoader;
    mImagePool = pImagePool;

    mImportFunction = importFunction;
    mAsyncJobs = asynchronousJobs;

    { // VULKAN Descriptor Set Stuff
        mGfxSession = session;

        VkDescriptorPoolSize size{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1024,
        };

        VkDescriptorPoolCreateInfo poolCI{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1024,
            .poolSizeCount = 1,
            .pPoolSizes = &size,
        };

        VkDevice device = foeGfxVkGetDevice(session);
        for (auto &it : mDescriptorPools) {
            VkResult vkRes = vkCreateDescriptorPool(device, &poolCI, nullptr, &it);
            if (vkRes != VK_SUCCESS) {
                errC = vkRes;
                goto INITIALIZATION_FAILED;
            }
        }
    }

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeMaterialLoader::deinitialize() {
    { // VULKAN Descriptor Set Stuff
        for (auto &it : mDescriptorPools) {
            if (it != VK_NULL_HANDLE)
                vkDestroyDescriptorPool(foeGfxVkGetDevice(mGfxSession), it, nullptr);

            it = VK_NULL_HANDLE;
        }
    }

    mImagePool = nullptr;
    mImageLoader = nullptr;
    mGfxFragmentDescriptorPool = nullptr;
    mShaderPool = nullptr;
    mShaderLoader = nullptr;

    mAsyncJobs = std::function<void(std::function<void()>)>{};
}

bool foeMaterialLoader::initialized() const noexcept { return mShaderLoader != nullptr; }

void foeMaterialLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == &mUnloadRequestLists[mUnloadRequestLists.size()]) {
        mCurrentUnloadRequests = &mUnloadRequestLists[0];
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        // Nothing to do yet
    }
}

void foeMaterialLoader::requestResourceLoad(foeMaterial *pMaterial) {
    ++mActiveJobs;
    mAsyncJobs([this, pMaterial] {
        loadResource(pMaterial);
        --mActiveJobs;
    });
}

void foeMaterialLoader::requestResourceUnload(foeMaterial *pMaterial) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pMaterial->dataWriteLock};

    // Only unload if it's 'loaded' and useCount is zero
    if (pMaterial->loadState == foeResourceLoadState::Loaded && pMaterial->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(std::move(pMaterial->data));

        pMaterial->data = {};
        pMaterial->loadState = foeResourceLoadState::Unloaded;
    }
}

#include <foe/graphics/vk/shader.hpp>
#include <foe/resource/shader.hpp>

VkDescriptorSet foeMaterialLoader::createDescriptorSet(foeMaterial *pMaterial,
                                                       uint32_t frameIndex) {
    if (pMaterial->data.subResources.pImage == nullptr) {
        return VK_NULL_HANDLE;
    }

    VkDescriptorSet set;
    auto vkDevice = foeGfxVkGetDevice(mGfxSession);
    vkResetDescriptorPool(vkDevice, mDescriptorPools[frameIndex], 0);

    // We have an image to do
    auto fragShader = pMaterial->data.subResources.pFragmentShader->getShader();

    auto descriptorSetLayout = foeGfxVkGetShaderDescriptorSetLayout(fragShader);

    VkDescriptorSetAllocateInfo setAI{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mDescriptorPools[frameIndex],
        .descriptorSetCount = 1U,
        .pSetLayouts = &descriptorSetLayout,
    };

    VkResult vkRes = vkAllocateDescriptorSets(vkDevice, &setAI, &set);
    if (vkRes != VK_SUCCESS) {
        std::abort();
    }

    VkDescriptorImageInfo imageInfo{
        .sampler = pMaterial->data.subResources.pImage->data.sampler,
        .imageView = pMaterial->data.subResources.pImage->data.view,
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

    return set;
}

void foeMaterialLoader::loadResource(foeMaterial *pMaterial) {
    // First, try to enter the 'loading' state
    auto expected = pMaterial->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pMaterial->loadState.compare_exchange_weak(expected, foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeMaterial {} in parrallel",
                static_cast<void *>(pMaterial))
        return;
    }

    std::error_code errC;
    foeGfxVkFragmentDescriptor *pNewFragDescriptor{nullptr};
    foeMaterial::SubResources subResources;
    foeResourceCreateInfoBase *pCreateInfo = nullptr;
    foeMaterialCreateInfo *createInfo = nullptr;

    bool read = mImportFunction(pMaterial->getID(), &pCreateInfo);
    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    createInfo = dynamic_cast<foeMaterialCreateInfo *>(pCreateInfo);
    if (createInfo == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    { // Resource Dependencies
        // Fragment Shader
        if (createInfo->fragmentShader != FOE_INVALID_ID) {
            subResources.pFragmentShader = mShaderPool->find(createInfo->fragmentShader);
            if (subResources.pFragmentShader == nullptr) {
                subResources.pFragmentShader =
                    new foeShader{createInfo->fragmentShader, mShaderLoader};
                if (!mShaderPool->add(subResources.pFragmentShader)) {
                    // Failed to add a 'new' shader, must've been added by another loading process
                    delete subResources.pFragmentShader;
                    subResources.pFragmentShader = mShaderPool->find(createInfo->fragmentShader);
                }
            }
            subResources.pFragmentShader->incrementRefCount();
            subResources.pFragmentShader->incrementUseCount();
        }

        // Image
        if (createInfo->image != FOE_INVALID_ID) {
            subResources.pImage = mImagePool->find(createInfo->image);

            if (subResources.pImage == nullptr) {
                subResources.pImage = new foeImage{createInfo->image, mImageLoader};

                if (!mImagePool->add(subResources.pImage)) {
                    delete subResources.pImage;

                    subResources.pImage = mImagePool->find(createInfo->image);
                }
            }

            subResources.pImage->incrementRefCount();
            subResources.pImage->incrementUseCount();
        }

        // Check all sub resources are loaded, if not, then leave and try to reload later.
        auto subResourcesState = subResources.getWorstSubresourceState();
        if (subResourcesState == foeResourceLoadState::Failed) {
            // The sub-resource failed to load itself, so set this as failed to load and
            // leave
            FOE_LOG(foeResource, Error,
                    "Failed to load foeMaterial '{}', as a sub resource failed to load",
                    foeIdToString(pMaterial->getID()))
            std::scoped_lock writeLock{pMaterial->dataWriteLock};

            // Reset the loading resources, no longer trying to load this
            pMaterial->loadingSubResources.reset();
            pMaterial->loadState = foeResourceLoadState::Failed;
            return;
        } else if (subResourcesState != foeResourceLoadState::Loaded) {
            // Something we depend upon isn't loaded itself, so leave and request ourselves
            // to attempt loading again
            std::scoped_lock writeLock{pMaterial->dataWriteLock};

            // Overwrite with the new sub-resources we're attempting to load.
            pMaterial->loadingSubResources = std::move(subResources);

            pMaterial->loadState = expected;
            requestResourceLoad(pMaterial);
            return;
        }
    }

    { // Using the sub-resources that are loaded, and definition data, create the resource
        foeGfxShader fragShader = (subResources.pFragmentShader != nullptr)
                                      ? subResources.pFragmentShader->getShader()
                                      : FOE_NULL_HANDLE;

        pNewFragDescriptor = mGfxFragmentDescriptorPool->get(
            (createInfo->hasRasterizationSCI) ? &createInfo->rasterizationSCI : nullptr,
            (createInfo->hasDepthStencilSCI) ? &createInfo->depthStencilSCI : nullptr,
            (createInfo->hasColourBlendSCI) ? &createInfo->colourBlendSCI : nullptr, fragShader);
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeMaterial {} with error {}:{}",
                static_cast<void *>(pMaterial), errC.value(), errC.message())

        pMaterial->loadingSubResources.reset();
        pMaterial->loadState = foeResourceLoadState::Failed;
    } else {
        foeMaterial::Data oldData;
        foeMaterial::Data newData{
            .subResources = std::move(subResources),
            .pGfxFragDescriptor = pNewFragDescriptor,
        };

        // Secure the resource, copy any old data out, and set the new data/state
        {
            std::scoped_lock writeLock{pMaterial->dataWriteLock};

            oldData = std::move(pMaterial->data);
            pMaterial->data = std::move(newData);
            pMaterial->loadingSubResources.reset();
            pMaterial->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        {
            std::scoped_lock unloadLock{mUnloadSync};
            mCurrentUnloadRequests->emplace_back(std::move(oldData));
        }
    }

    // No longer using the material reference, decrement.
    pMaterial->decrementRefCount();
}