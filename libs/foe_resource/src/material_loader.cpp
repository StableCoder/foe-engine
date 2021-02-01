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

#include <foe/resource/material.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeMaterialLoader::~foeMaterialLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeMaterialLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeMaterialLoader::initialize(
    foeFragmentDescriptorPool *pFragPool,
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

void foeMaterialLoader::deinitialize() { mFragPool = nullptr; }

bool foeMaterialLoader::initialized() const noexcept { return mFragPool != nullptr; }

void foeMaterialLoader::maintenance(foeGfxShader fragShader) {
    if (mLoadRequests.empty()) {
        return;
    }

    // Move the current request list to a local variable then unlock it, minimizing time with it
    // locked.
    mSync.lock();
    auto loadRequests = std::move(mLoadRequests);
    mSync.unlock();

    // Now we'll place all load requests into asynchronous jobs
    mActiveJobs += loadRequests.size();
    for (auto pMaterial : loadRequests) {
        // @todo : When asynchronous threading is implemented, change to it
        // mAsyncJobs([this, pMaterial, fragShader] {
        loadResource(pMaterial, fragShader);
        --mActiveJobs;
        // });
    }
}

void foeMaterialLoader::requestResourceLoad(foeMaterial *pMaterial) {
    std::scoped_lock lock{mSync};
    mLoadRequests.emplace_back(pMaterial);
}

void foeMaterialLoader::loadResource(foeMaterial *pMaterial, foeGfxShader fragShader) {
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
    auto pSourceData = pMaterial->pSourceData;
    foeFragmentDescriptor *pFragDescriptor{nullptr};

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

        pFragDescriptor = mFragPool->get(&rasterization, nullptr, &colourBlend, fragShader);
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeMaterial {} with error {}:{}",
                static_cast<void *>(pMaterial), errC.value(), errC.message())

        pMaterial->loadState = foeResourceLoadState::Failed;
    } else {
        std::scoped_lock writeLock{pMaterial->dataWriteLock};

        pMaterial->pLoadedSource = pSourceData.get();
        pMaterial->pFragDescriptor = pFragDescriptor;

        pMaterial->loadState = foeResourceLoadState::Loaded;
    }

    // No longer using the material reference, decrement.
    pMaterial->decrementRefCount();
}