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

#include <foe/graphics/resource/vertex_descriptor_loader.hpp>

#include <foe/graphics/resource/shader.hpp>
#include <foe/graphics/resource/shader_pool.hpp>
#include <foe/graphics/resource/type_defs.h>

#include "error_code.hpp"
#include "log.hpp"

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

std::error_code foeVertexDescriptorLoader::initialize(foeShaderPool *pShaderPool) {
    if (pShaderPool == nullptr) {
        return FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_LOADER_INITIALIZATION_FAILED;
    }

    std::error_code errC{FOE_GRAPHICS_RESOURCE_SUCCESS};

    mShaderPool = pShaderPool;

    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeVertexDescriptorLoader::deinitialize() { mShaderPool = nullptr; }

bool foeVertexDescriptorLoader::initialized() const noexcept {
    return mShaderPool != FOE_NULL_HANDLE;
}

void foeVertexDescriptorLoader::gfxMaintenance() {
    // Delayed Data Destroy

    // Unload Requests
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Load Requests
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    std::vector<LoadData> stillLoading;

    for (auto &it : toLoad) {
        auto subResLoadState =
            getWorstSubResourceState(it.data.pVertex, it.data.pTessellationControl,
                                     it.data.pTessellationEvaluation, it.data.pGeometry);

        if (subResLoadState == foeResourceState::Loaded) {
            if (it.data.pVertex != nullptr) {
                it.data.vertexDescriptor.mVertex = it.data.pVertex->data.shader;
            }
            if (it.data.pTessellationControl != nullptr) {
                it.data.vertexDescriptor.mTessellationControl =
                    it.data.pTessellationControl->data.shader;
            }
            if (it.data.pTessellationEvaluation != nullptr) {
                it.data.vertexDescriptor.mTessellationEvaluation =
                    it.data.pTessellationEvaluation->data.shader;
            }
            if (it.data.pGeometry != nullptr) {
                it.data.vertexDescriptor.mGeometry = it.data.pGeometry->data.shader;
            }

            auto *pCreateInfo =
                reinterpret_cast<foeVertexDescriptorCreateInfo *>(it.pCreateInfo.get());

            it.data.vertexDescriptor.mVertexInputSCI = pCreateInfo->vertexInputSCI;
            it.data.vertexDescriptor.mVertexInputBindings = pCreateInfo->inputBindings;
            it.data.vertexDescriptor.mVertexInputAttributes = pCreateInfo->inputAttributes;

            it.data.vertexDescriptor.mInputAssemblySCI = pCreateInfo->inputAssemblySCI;
            it.data.vertexDescriptor.mTessellationSCI = pCreateInfo->tessellationSCI;

            // Everything looks good, lock the resource and update it
            auto moveFn = [](void *pSrc, void *pDst) {
                auto *pSrcData = (foeVertexDescriptor *)pSrc;
                new (pDst) foeVertexDescriptor(std::move(*pSrcData));
            };

            it.pPostLoadFn(it.resource, {}, &it.data, moveFn, std::move(it.pCreateInfo), this,
                           foeVertexDescriptorLoader::unloadResource);

        } else if (subResLoadState == foeResourceState::Failed) {
            // At least one of the items failed to load
            if (it.data.pVertex != nullptr) {
                it.data.pVertex->decrementUseCount();
                it.data.pVertex->decrementRefCount();
            }
            if (it.data.pTessellationControl != nullptr) {
                it.data.pTessellationControl->decrementUseCount();
                it.data.pTessellationControl->decrementRefCount();
            }
            if (it.data.pTessellationEvaluation != nullptr) {
                it.data.pTessellationEvaluation->decrementUseCount();
                it.data.pTessellationEvaluation->decrementRefCount();
            }
            if (it.data.pGeometry != nullptr) {
                it.data.pGeometry->decrementUseCount();
                it.data.pGeometry->decrementRefCount();
            }

            it.pPostLoadFn(
                it.resource,
                foeToErrorCode(
                    FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD),
                nullptr, nullptr, nullptr, nullptr, nullptr);
        } else {
            // Sub-items are still at least loading
            stillLoading.emplace_back(std::move(it));
        }
    }

    // If there are items still loading, requeue them
    if (!stillLoading.empty()) {
        mLoadSync.lock();

        mLoadRequests.reserve(mLoadRequests.size() + stillLoading.size());
        for (auto &it : stillLoading) {
            mLoadRequests.emplace_back(std::move(it));
        }

        mLoadSync.unlock();
    }
}

bool foeVertexDescriptorLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeVertexDescriptorCreateInfo *>(pCreateInfo) != nullptr;
}

void foeVertexDescriptorLoader::load(void *pLoader,
                                     foeResource resource,
                                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                     PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeVertexDescriptorLoader *>(pLoader)->load(resource, pCreateInfo,
                                                                 pPostLoadFn);
}

void foeVertexDescriptorLoader::load(foeResource resource,
                                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                     PFN_foeResourcePostLoad *pPostLoadFn) {
    auto *pCI = dynamic_cast<foeVertexDescriptorCreateInfo *>(pCreateInfo.get());

    if (pCI == nullptr) {
        pPostLoadFn(resource, foeToErrorCode(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                    nullptr, nullptr, nullptr, nullptr, nullptr);
        return;
    }

    foeVertexDescriptor data{};

    // Find all of the subresources
    if (pCI->vertexShader != FOE_INVALID_ID) {
        data.pVertex = mShaderPool->findOrAdd(pCI->vertexShader);

        data.pVertex->incrementRefCount();
        data.pVertex->incrementUseCount();
        data.pVertex->loadResource(false);
    }
    if (pCI->tessellationControlShader != FOE_INVALID_ID) {
        data.pTessellationControl = mShaderPool->findOrAdd(pCI->tessellationControlShader);

        data.pTessellationControl->incrementRefCount();
        data.pTessellationControl->incrementUseCount();
        data.pTessellationControl->loadResource(false);
    }
    if (pCI->tessellationEvaluationShader != FOE_INVALID_ID) {
        data.pTessellationEvaluation = mShaderPool->findOrAdd(pCI->tessellationEvaluationShader);

        data.pTessellationEvaluation->incrementRefCount();
        data.pTessellationEvaluation->incrementUseCount();
        data.pTessellationEvaluation->loadResource(false);
    }
    if (pCI->geometryShader != FOE_INVALID_ID) {
        data.pGeometry = mShaderPool->findOrAdd(pCI->geometryShader);

        data.pGeometry->incrementRefCount();
        data.pGeometry->incrementUseCount();
        data.pGeometry->loadResource(false);
    }

    // Send to the loading queue
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .resource = resource,
        .pCreateInfo = pCreateInfo,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foeVertexDescriptorLoader::unloadResource(void *pContext,
                                               foeResource resource,
                                               uint32_t resourceIteration,
                                               PFN_foeResourceUnloadCall *pUnloadCallFn,
                                               bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeVertexDescriptorLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeVertexDescriptor *)pSrc;
            auto *pDstData = (foeVertexDescriptor *)pDst;

            *pDstData = std::move(*pSrcData);
            pSrcData->~foeVertexDescriptor();
        };

        foeVertexDescriptor data;

        if (!pUnloadCallFn(resource, resourceIteration, &data, moveFn)) {
            // If it failed, it's probably due to the resource iteration being different than
            // desired, so it didn't happen.
            return;
        }

        // Decrement the ref/use count of any sub-resources
        if (data.pVertex != nullptr) {
            data.pVertex->decrementUseCount();
            data.pVertex->decrementRefCount();
        }
        if (data.pTessellationControl != nullptr) {
            data.pTessellationControl->decrementUseCount();
            data.pTessellationControl->decrementRefCount();
        }
        if (data.pTessellationEvaluation != nullptr) {
            data.pTessellationEvaluation->decrementUseCount();
            data.pTessellationEvaluation->decrementRefCount();
        }
        if (data.pGeometry != nullptr) {
            data.pGeometry->decrementUseCount();
            data.pGeometry->decrementRefCount();
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