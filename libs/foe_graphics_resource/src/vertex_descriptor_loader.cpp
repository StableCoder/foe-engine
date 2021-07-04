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

#include <foe/graphics/resource/vertex_descriptor_loader.hpp>

#include <foe/resource/shader.hpp>
#include <foe/resource/shader_pool.hpp>

#include "error_code.hpp"
#include "log.hpp"

namespace {

template <typename SubResource, typename... SubResources>
foeResourceState getWorstSubResourceState(SubResource *pSubResource,
                                          SubResources *...pSubResources) {
    if constexpr (std::is_base_of<foeResourceBase, SubResource>::value) {
        if (pSubResource != nullptr) {
            auto state = pSubResource->getState();
            if (state != foeResourceState::Loaded) {
                return state;
            }
        }
    } else {
        if (pSubResource != nullptr) {
            auto state = pSubResource->getLoadState();
            if (state != foeResourceLoadState::Loaded) {
                return foeResourceState::Unloaded;
            }
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

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeVertexDescriptorLoader::deinitialize() { mShaderPool = nullptr; }

void foeVertexDescriptorLoader::gfxMaintenance() {
    // Delayed Data Destroy

    // Unload Requests
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.pResource, it.iteration, true);
        it.pResource->decrementRefCount();
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

            auto *pCI =
                reinterpret_cast<foeVertexDescriptorCreateInfo *>(it.data.pCreateInfo.get());

            it.data.vertexDescriptor.mVertexInputSCI = pCI->vertexInputSCI;
            it.data.vertexDescriptor.mVertexInputBindings = pCI->inputBindings;
            it.data.vertexDescriptor.mVertexInputAttributes = pCI->inputAttributes;

            it.data.vertexDescriptor.mInputAssemblySCI = pCI->inputAssemblySCI;
            it.data.vertexDescriptor.mTessellationSCI = pCI->tessellationSCI;

            // Everything looks good, lock the resource and update it
            it.pResource->modifySync.lock();

            if (it.pResource->data.pUnloadFn != nullptr) {
                it.pResource->data.pUnloadFn(it.pResource->data.pUnloadContext, it.pResource,
                                             it.pResource->iteration, true);
            }

            ++it.pResource->iteration;
            it.pResource->data = std::move(it.data);
            it.pPostLoadFn(it.pResource, {});

            it.pResource->modifySync.unlock();
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
                it.pResource,
                FOE_GRAPHICS_RESOURCE_ERROR_VERTEX_DESCRIPTOR_SUBRESOURCE_FAILED_TO_LOAD);
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

void foeVertexDescriptorLoader::load(void *pResource,
                                     std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                     void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pVertexDescriptor = reinterpret_cast<foeVertexDescriptor *>(pResource);
    auto *pCI = reinterpret_cast<foeVertexDescriptorCreateInfo *>(pCreateInfo.get());

    foeVertexDescriptor::Data data{
        .pCreateInfo = pCreateInfo,
    };

    // Find all of the subresources
    if (pCI->vertexShader != FOE_INVALID_ID) {
        data.pVertex = mShaderPool->findOrAdd(pCI->vertexShader);

        data.pVertex->incrementRefCount();
        data.pVertex->incrementUseCount();
        data.pVertex->requestLoad();
    }
    if (pCI->tessellationControlShader != FOE_INVALID_ID) {
        data.pTessellationControl = mShaderPool->findOrAdd(pCI->tessellationControlShader);

        data.pTessellationControl->incrementRefCount();
        data.pTessellationControl->incrementUseCount();
        data.pTessellationControl->requestLoad();
    }
    if (pCI->tessellationEvaluationShader != FOE_INVALID_ID) {
        data.pTessellationEvaluation = mShaderPool->findOrAdd(pCI->tessellationEvaluationShader);

        data.pTessellationEvaluation->incrementRefCount();
        data.pTessellationEvaluation->incrementUseCount();
        data.pTessellationEvaluation->requestLoad();
    }
    if (pCI->geometryShader != FOE_INVALID_ID) {
        data.pGeometry = mShaderPool->findOrAdd(pCI->geometryShader);

        data.pGeometry->incrementRefCount();
        data.pGeometry->incrementUseCount();
        data.pGeometry->requestLoad();
    }

    // Send to the loading queue
    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .pResource = pVertexDescriptor,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foeVertexDescriptorLoader::unloadResource(void *pContext,
                                               void *pResource,
                                               uint32_t resourceIteration,
                                               bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeVertexDescriptorLoader *>(pContext);
    auto *pVertexDescriptor = reinterpret_cast<foeVertexDescriptor *>(pResource);

    if (immediateUnload) {
        pVertexDescriptor->modifySync.lock();

        if (pVertexDescriptor->iteration == resourceIteration) {
            auto data = std::move(pVertexDescriptor->data);

            pVertexDescriptor->data = {};
            pVertexDescriptor->state = foeResourceState::Unloaded;
            ++pVertexDescriptor->iteration;

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
        }

        pVertexDescriptor->modifySync.unlock();
    } else {
        pVertexDescriptor->incrementRefCount();
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pResource = pVertexDescriptor,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadSync.unlock();
    }
}