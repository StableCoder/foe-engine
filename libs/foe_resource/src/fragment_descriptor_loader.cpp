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

#include <foe/resource/fragment_descriptor_loader.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeFragmentDescriptorLoader::~foeFragmentDescriptorLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal,
                "foeFragmentDescriptorLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeFragmentDescriptorLoader::initialize(
    foeGfxVkFragmentDescriptorPool *pFragPool,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mFragPool = pFragPool;
    mAsyncJobs = asynchronousJobs;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeFragmentDescriptorLoader::deinitialize() { mFragPool = nullptr; }

bool foeFragmentDescriptorLoader::initialized() const noexcept { return mFragPool != nullptr; }

void foeFragmentDescriptorLoader::processLoadRequests(foeGfxShader fragShader) {
    if (mLoadRequests.empty()) {
        return;
    }

    // Move the current request list to a local variable then unlock it, minimizing time with it
    // locked.
    mLoadSync.lock();
    auto loadRequests = std::move(mLoadRequests);
    mLoadSync.unlock();

    // Now we'll place all load requests into asynchronous jobs
    mActiveJobs += loadRequests.size();
    for (auto pFragDescriptor : loadRequests) {
        mAsyncJobs([this, pFragDescriptor, fragShader] {
            loadResource(pFragDescriptor, fragShader);
            --mActiveJobs;
        });
    }
}

void foeFragmentDescriptorLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == &mUnloadRequestLists[mUnloadRequestLists.size()]) {
        mCurrentUnloadRequests = &mUnloadRequestLists[0];
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        // @todo Implement foeFragmentDescriptor unloading when stuff to unload (after shader added)
    }
}

void foeFragmentDescriptorLoader::requestResourceLoad(foeFragmentDescriptor *pFragDescriptor) {
    std::scoped_lock lock{mLoadSync};
    mLoadRequests.emplace_back(pFragDescriptor);
}

void foeFragmentDescriptorLoader::requestResourceUnload(foeFragmentDescriptor *pFragDescriptor) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pFragDescriptor->dataWriteLock};

    if (pFragDescriptor->loadState == foeResourceLoadState::Loaded) {
        mCurrentUnloadRequests->emplace_back(pFragDescriptor->data);

        pFragDescriptor->data = {};
        pFragDescriptor->loadState = foeResourceLoadState::Unloaded;
    }
}

void foeFragmentDescriptorLoader::loadResource(foeFragmentDescriptor *pFragDescriptor,
                                               foeGfxShader fragShader) {
    // First, try to enter the 'loading' state
    auto expected = pFragDescriptor->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pFragDescriptor->loadState.compare_exchange_weak(expected,
                                                             foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeFragmentDescriptor {} in parrallel",
                static_cast<void *>(pFragDescriptor))
        return;
    }

    std::error_code errC;
    auto pSourceData = pFragDescriptor->pSourceData;
    foeGfxVkFragmentDescriptor *pNewFragDescriptor{nullptr};

    {
        auto rasterization = VkPipelineRasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .lineWidth = 1.0f,
        };
        std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments{
            VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            }};
        auto colourBlend = VkPipelineColorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<uint32_t>(colourBlendAttachments.size()),
            .pAttachments = colourBlendAttachments.data(),
        };

        pNewFragDescriptor = mFragPool->get(&rasterization, nullptr, &colourBlend, fragShader);
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeFragmentDescriptor {} with error {}:{}",
                static_cast<void *>(pFragDescriptor), errC.value(), errC.message())

        pFragDescriptor->loadState = foeResourceLoadState::Failed;
    } else {
        foeFragmentDescriptor::Data oldData;
        foeFragmentDescriptor::Data newData{
            .pLoadedSource = pSourceData.get(),
            .pGfxFragDescriptor = pNewFragDescriptor,
        };

        // Secure the resource, copy any old data out, and set the new data/state
        {
            std::scoped_lock writeLock{pFragDescriptor->dataWriteLock};

            oldData = pFragDescriptor->data;
            pFragDescriptor->data = newData;
            pFragDescriptor->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        if (oldData.pLoadedSource != nullptr) {
            std::scoped_lock unloadLock{mUnloadSync};
            mCurrentUnloadRequests->emplace_back(pFragDescriptor->data);
        }
    }

    // No longer using the reference, decrement.
    pFragDescriptor->decrementRefCount();
}