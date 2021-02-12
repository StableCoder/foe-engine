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

#include <foe/resource/fragment_descriptor.hpp>
#include <foe/resource/fragment_descriptor_pool.hpp>
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
    foeFragmentDescriptorLoader *pFragmentDescriptorLoader,
    foeFragmentDescriptorPool *pFragmentDescriptorPool,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mFragmentDescriptorLoader = pFragmentDescriptorLoader;
    mFragmentDescriptorPool = pFragmentDescriptorPool;
    mAsyncJobs = asynchronousJobs;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeMaterialLoader::deinitialize() {
    mAsyncJobs = std::function<void(std::function<void()>)>{};
}

bool foeMaterialLoader::initialized() const noexcept { return static_cast<bool>(mAsyncJobs); }

void foeMaterialLoader::processLoadRequests() {
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
    for (auto pMaterial : loadRequests) {
        mAsyncJobs([this, pMaterial] {
            loadResource(pMaterial);
            --mActiveJobs;
        });
    }
}

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
    std::scoped_lock lock{mLoadSync};
    mLoadRequests.emplace_back(pMaterial);
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

#include <foe/resource/imex/material.hpp>

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
    std::string fragmentDescriptorName;
    foeMaterial::SubResources subResources;

    bool read = import_material_definition(pMaterial->getName(), fragmentDescriptorName);
    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    { // Resource Dependencies
        // Fragment Descriptor
        if (!fragmentDescriptorName.empty()) {
            subResources.pFragmentDescriptor =
                mFragmentDescriptorPool->find(fragmentDescriptorName);
            if (subResources.pFragmentDescriptor == nullptr) {
                subResources.pFragmentDescriptor =
                    new foeFragmentDescriptor{fragmentDescriptorName, mFragmentDescriptorLoader};
                if (!mFragmentDescriptorPool->add(subResources.pFragmentDescriptor)) {
                    // Failed to add a 'new' shader, must've been added by another loading process
                    delete subResources.pFragmentDescriptor;
                    subResources.pFragmentDescriptor =
                        mFragmentDescriptorPool->find(fragmentDescriptorName);
                }
            }
            subResources.pFragmentDescriptor->incrementRefCount();
            subResources.pFragmentDescriptor->incrementUseCount();
        }

        // Check all sub resources are loaded, if not, then leave and try to reload later.
        if (subResources.pFragmentDescriptor != nullptr) {
            if (subResources.pFragmentDescriptor->getLoadState() == foeResourceLoadState::Failed) {
                // The sub-resource failed to load itself, so set this as failed to load and leave
                FOE_LOG(foeResource, Error,
                        "Failed to load foeMaterial '{}', as sub resource foeFragmentDescriptor {} "
                        "failed to load",
                        pMaterial->getName(), subResources.pFragmentDescriptor->getName())
                std::scoped_lock writeLock{pMaterial->dataWriteLock};

                // Reset the loading resources, no longer trying to load this
                pMaterial->loadingSubResources.reset();
                pMaterial->loadState = foeResourceLoadState::Failed;
                return;
            } else if (subResources.pFragmentDescriptor->getLoadState() !=
                       foeResourceLoadState::Loaded) {
                // Something we depend upon isn't loaded itself, so leave and request ourselves to
                // attempt loading again
                std::scoped_lock writeLock{pMaterial->dataWriteLock};

                // Overwrite with the new sub-resources we're attempting to load.
                pMaterial->loadingSubResources = std::move(subResources);

                pMaterial->loadState = expected;
                requestResourceLoad(pMaterial);
                return;
            }
        }
    }

    { // Using the sub-resources that are loaded, and definition data, create the resource
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