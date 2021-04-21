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

#include <foe/resource/vertex_descriptor_loader.hpp>

#include <foe/resource/shader.hpp>
#include <foe/resource/shader_pool.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeVertexDescriptorLoader::~foeVertexDescriptorLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal,
                "foeVertexDescriptorLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeVertexDescriptorLoader::initialize(
    foeShaderLoader *pShaderLoader,
    foeShaderPool *pShaderPool,
    std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mShaderLoader = pShaderLoader;
    mShaderPool = pShaderPool;

    mImportFunction = importFunction;
    mAsyncJobs = asynchronousJobs;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeVertexDescriptorLoader::deinitialize() {
    mShaderPool = nullptr;
    mShaderLoader = nullptr;
}

bool foeVertexDescriptorLoader::initialized() const noexcept { return mShaderLoader != nullptr; }

void foeVertexDescriptorLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == mUnloadRequestLists.end()) {
        mCurrentUnloadRequests = mUnloadRequestLists.begin();
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        // Nothing to do, compiles out
    }
}

void foeVertexDescriptorLoader::requestResourceLoad(foeVertexDescriptor *pVertexDescriptor) {
    ++mActiveJobs;
    mAsyncJobs([this, pVertexDescriptor] {
        loadResource(pVertexDescriptor);
        --mActiveJobs;
    });
}

void foeVertexDescriptorLoader::requestResourceUnload(foeVertexDescriptor *pVertexDescriptor) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pVertexDescriptor->dataWriteLock};

    // Only unload if it's 'loaded' and useCount is zero
    if (pVertexDescriptor->loadState == foeResourceLoadState::Loaded &&
        pVertexDescriptor->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(std::move(pVertexDescriptor->data));

        pVertexDescriptor->data = {};
        pVertexDescriptor->loadState = foeResourceLoadState::Unloaded;
    }
}

void foeVertexDescriptorLoader::loadResource(foeVertexDescriptor *pVertexDescriptor) {
    auto expected = pVertexDescriptor->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pVertexDescriptor->loadState.compare_exchange_weak(expected,
                                                               foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeVertexDescriptor {} in parrallel",
                static_cast<void *>(pVertexDescriptor))
        return;
    }

    std::error_code errC;
    foeVertexDescriptor::SubResources newSubResources{};
    foeGfxVertexDescriptor theVertexDescriptor;
    foeVertexDescriptorCreateInfo *pVertexDescriptorCI = nullptr;

    std::unique_ptr<foeResourceCreateInfoBase> createInfo{
        mImportFunction(pVertexDescriptor->getID())};
    if (createInfo == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    pVertexDescriptorCI = dynamic_cast<foeVertexDescriptorCreateInfo *>(createInfo.get());
    if (pVertexDescriptorCI == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    { // Resource Dependencies
        if (pVertexDescriptorCI->vertexShader != FOE_INVALID_ID) {
            newSubResources.pVertex = mShaderPool->find(pVertexDescriptorCI->vertexShader);
            if (newSubResources.pVertex == nullptr) {
                newSubResources.pVertex =
                    new foeShader{pVertexDescriptorCI->vertexShader, mShaderLoader};
                if (!mShaderPool->add(newSubResources.pVertex)) {
                    // Failed to add a 'new' shader, must've been added by another loading process
                    delete newSubResources.pVertex;
                    newSubResources.pVertex = mShaderPool->find(pVertexDescriptorCI->vertexShader);
                }
            }
            newSubResources.pVertex->incrementRefCount();
            newSubResources.pVertex->incrementUseCount();
        }

        // Check all sub resources are loaded, if not, then leave and try to reload later.
        if (newSubResources.pVertex != nullptr) {
            if (newSubResources.pVertex->getLoadState() == foeResourceLoadState::Failed) {
                // The sub-resource failed to load itself, so set this as failed to load and leave
                FOE_LOG(foeResource, Error,
                        "Failed to load foeFragmentDescriptor {}, as sub resource foeShader {} "
                        "failed to load",
                        static_cast<void *>(pVertexDescriptor),
                        static_cast<void *>(newSubResources.pVertex))
                std::scoped_lock writeLock{pVertexDescriptor->dataWriteLock};

                // Reset the loading resources, no longer trying to load this
                pVertexDescriptor->loading.reset();
                pVertexDescriptor->loadState = foeResourceLoadState::Failed;
                return;
            } else if (newSubResources.pVertex->getLoadState() != foeResourceLoadState::Loaded) {
                // Something we depend upon isn't loaded itself, so leave and request ourselves to
                // attempt loading again
                std::scoped_lock writeLock{pVertexDescriptor->dataWriteLock};

                // Overwrite with the new sub-resources we're attempting to load.
                pVertexDescriptor->loading = std::move(newSubResources);

                pVertexDescriptor->loadState = expected;
                requestResourceLoad(pVertexDescriptor);
                return;
            }
        }
    }

    { // Using the loaded sub-resources and definition data, create the resource
        if (newSubResources.pVertex != nullptr)
            theVertexDescriptor.mVertex = newSubResources.pVertex->getShader();

        if (newSubResources.pTessellationControl != nullptr)
            theVertexDescriptor.mTessellationControl =
                newSubResources.pTessellationControl->getShader();

        if (newSubResources.pTessellationEvaluation != nullptr)
            theVertexDescriptor.mTessellationEvaluation =
                newSubResources.pTessellationEvaluation->getShader();

        if (newSubResources.pGeometry != nullptr)
            theVertexDescriptor.mGeometry = newSubResources.pGeometry->getShader();

        theVertexDescriptor.mVertexInputSCI = pVertexDescriptorCI->vertexInputSCI;
        theVertexDescriptor.mVertexInputBindings = std::move(pVertexDescriptorCI->inputBindings);
        theVertexDescriptor.mVertexInputAttributes =
            std::move(pVertexDescriptorCI->inputAttributes);

        theVertexDescriptor.mInputAssemblySCI = pVertexDescriptorCI->inputAssemblySCI;

        theVertexDescriptor.mTessellationSCI = pVertexDescriptorCI->tessellationSCI;
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeVertexDescriptor {} with error {}:{}",
                static_cast<void *>(pVertexDescriptor), errC.value(), errC.message())

        pVertexDescriptor->loading.reset();

        pVertexDescriptor->loadState = foeResourceLoadState::Failed;
    } else {
        foeVertexDescriptor::Data oldData;
        foeVertexDescriptor::Data newData{
            .subResources = std::move(newSubResources),
            .gfxVertDescriptor = theVertexDescriptor,
        };

        // Secure the resource, copy any old data out, and set the new data/state
        {
            std::scoped_lock writeLock{pVertexDescriptor->dataWriteLock};

            oldData = std::move(pVertexDescriptor->data);
            pVertexDescriptor->data = std::move(newData);
            pVertexDescriptor->loading.reset();
            pVertexDescriptor->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        {
            std::scoped_lock unloadLock{mUnloadSync};
            mCurrentUnloadRequests->emplace_back(std::move(oldData));
        }
    }

    // No longer using the reference, decrement.
    pVertexDescriptor->decrementRefCount();
}
