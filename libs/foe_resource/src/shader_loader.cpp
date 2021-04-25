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

#include <foe/resource/shader_loader.hpp>

#include "error_code.hpp"
#include "log.hpp"

foeShaderLoader::~foeShaderLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeShaderLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeShaderLoader::initialize(
    foeGfxSession gfxSession,
    std::function<foeResourceCreateInfoBase *(foeId)> importFunction,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mGfxSession = gfxSession;

    mImportFunction = importFunction;
    mAsyncJobs = asynchronousJobs;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeShaderLoader::deinitialize() { mGfxSession = FOE_NULL_HANDLE; }

bool foeShaderLoader::initialized() const noexcept { return mGfxSession != FOE_NULL_HANDLE; }

void foeShaderLoader::processUnloadRequests() {
    mUnloadSync.lock();
    ++mCurrentUnloadRequests;
    if (mCurrentUnloadRequests == mUnloadRequestLists.end()) {
        mCurrentUnloadRequests = mUnloadRequestLists.begin();
    }
    auto unloadRequests = std::move(*mCurrentUnloadRequests);
    mUnloadSync.unlock();

    for (auto &data : unloadRequests) {
        if (data.shader != FOE_NULL_HANDLE)
            foeGfxDestroyShader(mGfxSession, data.shader);
    }
}

void foeShaderLoader::requestResourceLoad(foeShader *pShader) {
    ++mActiveJobs;
    mAsyncJobs([this, pShader] {
        loadResource(pShader);
        --mActiveJobs;
    });
}

void foeShaderLoader::requestResourceUnload(foeShader *pShader) {
    std::scoped_lock unloadLock{mUnloadSync};
    std::scoped_lock writeLock{pShader->dataWriteLock};

    // Only unload if it's 'loaded' and useCount is zero
    if (pShader->loadState == foeResourceLoadState::Loaded && pShader->getUseCount() == 0) {
        mCurrentUnloadRequests->emplace_back(pShader->data);

        pShader->data = {};
        pShader->loadState = foeResourceLoadState::Unloaded;
    }
}

#include "external_shader.hpp"
#include <foe/graphics/vk/session.hpp>
#include <foe/graphics/vk/shader.hpp>

void foeShaderLoader::loadResource(foeShader *pShader) {
    // First, try to enter the 'loading' state
    auto expected = pShader->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pShader->loadState.compare_exchange_weak(expected, foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeShader {} in parrallel",
                static_cast<void *>(pShader))
        return;
    }

    std::error_code errC;
    foeGfxShader newShader{FOE_NULL_HANDLE};
    foeShaderCreateInfo *pShaderCI = nullptr;

    // Read in the definition
    std::unique_ptr<foeResourceCreateInfoBase> createInfo{mImportFunction(pShader->getID())};
    if (createInfo == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    pShaderCI = dynamic_cast<foeShaderCreateInfo *>(createInfo.get());
    if (pShaderCI == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    {
        auto shaderCode = loadShaderDataFromFile(pShaderCI->shaderCodeFile);
        errC = foeGfxVkCreateShader(mGfxSession, pShaderCI->builtinSetLayouts, shaderCode.size(),
                                    reinterpret_cast<uint32_t *>(shaderCode.data()),
                                    &pShaderCI->descriptorSetLayoutCI, pShaderCI->pushConstantRange,
                                    &newShader);
        if (errC) {
            goto LOADING_FAILED;
        }
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeShader {} with error {}:{}",
                static_cast<void *>(pShader), errC.value(), errC.message())

        pShader->loadState = foeResourceLoadState::Failed;
    } else {
        foeShader::Data oldData;
        foeShader::Data newData{
            .shader = newShader,
        };

        // Secure the resource, copy any old data out, and set the new data/state
        {
            std::scoped_lock writeLock{pShader->dataWriteLock};

            pShader->createInfo.reset(
                reinterpret_cast<foeShaderCreateInfo *>(createInfo.release()));

            oldData = pShader->data;
            pShader->data = newData;
            pShader->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        {
            std::scoped_lock unloadLock{mUnloadSync};
            mCurrentUnloadRequests->emplace_back(oldData);
        }
    }

    // No longer using the reference, decrement.
    pShader->decrementRefCount();
}