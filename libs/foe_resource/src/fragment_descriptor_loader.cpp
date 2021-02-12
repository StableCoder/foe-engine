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

#include <foe/resource/shader.hpp>
#include <foe/resource/shader_pool.hpp>

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
    foeShaderLoader *pShaderLoader,
    foeShaderPool *pShaderPool,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mFragPool = pFragPool;
    mShaderLoader = pShaderLoader;
    mShaderPool = pShaderPool;
    mAsyncJobs = asynchronousJobs;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeFragmentDescriptorLoader::deinitialize() {
    mShaderPool = nullptr;
    mShaderLoader = nullptr;
    mFragPool = nullptr;
}

bool foeFragmentDescriptorLoader::initialized() const noexcept { return mFragPool != nullptr; }

void foeFragmentDescriptorLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == &mUnloadRequestLists[mUnloadRequestLists.size()]) {
        mCurrentUnloadRequests = &mUnloadRequestLists[0];
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        // Nothing to do, compiles out
    }
}

void foeFragmentDescriptorLoader::requestResourceLoad(foeFragmentDescriptor *pFragDescriptor) {
    ++mActiveJobs;
    mAsyncJobs([this, pFragDescriptor] {
        loadResource(pFragDescriptor);
        --mActiveJobs;
    });
}

void foeFragmentDescriptorLoader::requestResourceUnload(foeFragmentDescriptor *pFragDescriptor) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pFragDescriptor->dataWriteLock};

    // Only unload if it's 'loaded' and useCount is zero
    if (pFragDescriptor->loadState == foeResourceLoadState::Loaded &&
        pFragDescriptor->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(std::move(pFragDescriptor->data));

        pFragDescriptor->data = {};
        pFragDescriptor->loadState = foeResourceLoadState::Unloaded;
    }
}

#include <foe/resource/imex/fragment_descriptor.hpp>

void foeFragmentDescriptorLoader::loadResource(foeFragmentDescriptor *pFragDescriptor) {
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
    foeGfxVkFragmentDescriptor *pNewFragDescriptor{nullptr};

    std::string fragmentShader;

    bool hasRasterizationSCI = false;
    VkPipelineRasterizationStateCreateInfo rasterizationSCI;
    bool hasDepthStencilSCI = false;
    VkPipelineDepthStencilStateCreateInfo depthStencilSCI;
    bool hasColourBlendSCI = false;
    VkPipelineColorBlendStateCreateInfo colourBlendSCI;
    std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments;
    foeFragmentDescriptor::SubResources loadingSubResources{};

    // Read in the definition
    bool read = import_fragment_descriptor_definition(
        pFragDescriptor->getName(), fragmentShader, hasRasterizationSCI, rasterizationSCI,
        hasDepthStencilSCI, depthStencilSCI, hasColourBlendSCI, colourBlendSCI,
        colourBlendAttachments);
    colourBlendSCI.pAttachments = colourBlendAttachments.data();
    colourBlendSCI.attachmentCount = colourBlendAttachments.size();

    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    { //  Resource Dependencies
        // Fragment Shader
        if (!fragmentShader.empty()) {
            loadingSubResources.pFragmentShader = mShaderPool->find(fragmentShader);
            if (loadingSubResources.pFragmentShader == nullptr) {
                loadingSubResources.pFragmentShader = new foeShader{fragmentShader, mShaderLoader};
                if (!mShaderPool->add(loadingSubResources.pFragmentShader)) {
                    // Failed to add a 'new' shader, must've been added by another loading process
                    delete loadingSubResources.pFragmentShader;
                    loadingSubResources.pFragmentShader = mShaderPool->find(fragmentShader);
                }
            }
            loadingSubResources.pFragmentShader->incrementRefCount();
            loadingSubResources.pFragmentShader->incrementUseCount();
        }

        // Check all sub resources are loaded, if not, then leave and try to reload later.
        if (loadingSubResources.pFragmentShader != nullptr) {
            if (loadingSubResources.pFragmentShader->getLoadState() ==
                foeResourceLoadState::Failed) {
                // The sub-resource failed to load itself, so set this as failed to load and leave
                FOE_LOG(foeResource, Error,
                        "Failed to load foeFragmentDescriptor {}, as sub resource foeShader {} "
                        "failed to load",
                        static_cast<void *>(pFragDescriptor),
                        static_cast<void *>(loadingSubResources.pFragmentShader))
                std::scoped_lock writeLock{pFragDescriptor->dataWriteLock};

                // Reset the loading resources, no longer trying to load this
                pFragDescriptor->loading.reset();
                pFragDescriptor->loadState = foeResourceLoadState::Failed;
                return;
            } else if (loadingSubResources.pFragmentShader->getLoadState() !=
                       foeResourceLoadState::Loaded) {
                // Something we depend upon isn't loaded itself, so leave and request ourselves to
                // attempt loading again
                std::scoped_lock writeLock{pFragDescriptor->dataWriteLock};

                // Overwrite with the new sub-resources we're attempting to load.
                pFragDescriptor->loading = std::move(loadingSubResources);

                pFragDescriptor->loadState = expected;
                requestResourceLoad(pFragDescriptor);
                return;
            }
        }
    }

    { // Using the sub-resources that are loaded, and the definition data, create the resource
        foeGfxShader fragShader = (loadingSubResources.pFragmentShader != nullptr)
                                      ? loadingSubResources.pFragmentShader->getShader()
                                      : FOE_NULL_HANDLE;

        pNewFragDescriptor =
            mFragPool->get((hasRasterizationSCI) ? &rasterizationSCI : nullptr,
                           (hasDepthStencilSCI) ? &depthStencilSCI : nullptr,
                           (hasColourBlendSCI) ? &colourBlendSCI : nullptr, fragShader);
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeFragmentDescriptor {} with error {}:{}",
                static_cast<void *>(pFragDescriptor), errC.value(), errC.message())

        pFragDescriptor->loading.reset();

        pFragDescriptor->loadState = foeResourceLoadState::Failed;
    } else {
        foeFragmentDescriptor::Data oldData;
        foeFragmentDescriptor::Data newData{
            .loaded = std::move(loadingSubResources),
            .pGfxFragDescriptor = pNewFragDescriptor,
        };

        // Secure the resource, copy any old data out, and set the new data/state
        {
            std::scoped_lock writeLock{pFragDescriptor->dataWriteLock};

            oldData = std::move(pFragDescriptor->data);
            pFragDescriptor->data = std::move(newData);
            pFragDescriptor->loading.reset();
            pFragDescriptor->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        {
            std::scoped_lock unloadLock{mUnloadSync};
            mCurrentUnloadRequests->emplace_back(std::move(oldData));
        }
    }

    // No longer using the reference, decrement.
    pFragDescriptor->decrementRefCount();
}