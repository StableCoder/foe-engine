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

void foeFragmentDescriptorLoader::processLoadRequests() {
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
        mAsyncJobs([this, pFragDescriptor] {
            loadResource(pFragDescriptor);
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

    // Only unload if it's 'loaded' and useCount is zero
    if (pFragDescriptor->loadState == foeResourceLoadState::Loaded &&
        pFragDescriptor->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(pFragDescriptor->data);

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
    auto pSourceData = pFragDescriptor->pSourceData;
    foeGfxVkFragmentDescriptor *pNewFragDescriptor{nullptr};

    std::string fragmentShader;

    VkPipelineRasterizationStateCreateInfo rasterizationSCI;
    VkPipelineDepthStencilStateCreateInfo depthStencilSCI;
    std::vector<VkPipelineColorBlendAttachmentState> colourBlendAttachments;
    VkPipelineColorBlendStateCreateInfo colourBlendSCI;

    // Read in the definition
    bool read = import_fragment_descriptor_definition(pFragDescriptor->getName(), fragmentShader,
                                                      rasterizationSCI, depthStencilSCI,
                                                      colourBlendAttachments, colourBlendSCI);
    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    {
        foeShader *pFragmentShader{nullptr};
        if (!fragmentShader.empty()) {
            pFragmentShader = mShaderPool->find(fragmentShader);
            if (pFragmentShader == nullptr) {
                pFragmentShader = new foeShader{fragmentShader, mShaderLoader};
                if (!mShaderPool->add(pFragmentShader)) {
                    delete pFragmentShader;
                    pFragmentShader = mShaderPool->find(fragmentShader);
                }
            }
        }

        if (pFragDescriptor->pShader != pFragmentShader) {
            auto oldFragShader = pFragDescriptor->pShader;
            pFragmentShader->incrementUseCount();
            pFragDescriptor->pShader = pFragmentShader;
            if (oldFragShader != nullptr) {
                oldFragShader->decrementUseCount();
            }
        }

        // If resources we depend on aren't loaded, then leave, and try to re-load later
        if (pFragDescriptor->pShader != nullptr &&
            pFragDescriptor->pShader->getLoadState() != foeResourceLoadState::Loaded) {
            // Something we depend upon isn't loaded itself, so leave and request ourselves to
            // attempt loading again
            pFragDescriptor->loadState = expected;
            requestResourceLoad(pFragDescriptor);
            return;
        }

        foeGfxShader fragShader = (pFragDescriptor->pShader != nullptr)
                                      ? pFragDescriptor->pShader->getShader()
                                      : FOE_NULL_HANDLE;

        pNewFragDescriptor =
            mFragPool->get(&rasterizationSCI, nullptr, &colourBlendSCI, fragShader);
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