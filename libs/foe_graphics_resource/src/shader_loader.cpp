// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "shader_loader.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/shader_create_info.h>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/shader.h>

#include "log.hpp"
#include "result.h"

foeResultSet foeShaderLoader::initialize(
    foeResourcePool resourcePool,
    std::function<foeResultSet(char const *, foeManagedMemory *)> externalFileSearchFn) {
    if (resourcePool == FOE_NULL_HANDLE || !externalFileSearchFn)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED);

    mResourcePool = resourcePool;
    mExternalFileSearchFn = externalFileSearchFn;

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeShaderLoader::deinitialize() { mExternalFileSearchFn = {}; }

bool foeShaderLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

foeResultSet foeShaderLoader::initializeGraphics(foeGfxSession gfxSession) {
    if (!initialized())
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_NOT_INITIALIZED);

    mGfxSession = gfxSession;

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeShaderLoader::deinitializeGraphics() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork = foeResourcePoolUnloadType(mResourcePool,
                                                 FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER) > 0;

        gfxMaintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();

        mDestroySync.lock();
        for (auto const &it : mDataDestroyLists) {
            upcomingWork |= !it.empty();
        }
        mDestroySync.unlock();
    } while (upcomingWork);

    // External
    mGfxSession = FOE_NULL_HANDLE;
}

bool foeShaderLoader::initializedGraphics() const noexcept {
    return mGfxSession != FOE_NULL_HANDLE;
}

void foeShaderLoader::gfxMaintenance() {
    // Destruction
    mDestroySync.lock();
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }
    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);
    mDestroySync.unlock();

    for (auto &it : toDestroy) {
        if (it.shader != FOE_NULL_HANDLE) {
            foeGfxDestroyShader(mGfxSession, it.shader);
        }
    }

    // Unloads
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.unloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Loads
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        auto moveFn = [](void *pSrc, void *pDst) {
            foeShader *pSrcData = (foeShader *)pSrc;
            new (pDst) foeShader(std::move(*pSrcData));
        };

        if (foeResourceGetType(it.resource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
            // We need to replace this placeholder with the actual resource
            foeResource newResource = foeResourcePoolLoadedReplace(
                mResourcePool, foeResourceGetID(it.resource),
                FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER, sizeof(foeShader), &it.data, moveFn,
                this, foeShaderLoader::unloadResource);

            if (newResource == FOE_NULL_HANDLE)
                // @TODO - Handle failure
                std::abort();

            // Decrement references we no longer need/use
            foeResourceDecrementRefCount(it.resource);
            foeResourceDecrementRefCount(newResource);
        } else {
            // The resource handle is of the desired type already, use the regular load function
            it.postLoadFn(it.resource, to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS), &it.data,
                          moveFn, this, foeShaderLoader::unloadResource);
        }
    }
}

bool foeShaderLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO;
}

void foeShaderLoader::load(void *pLoader,
                           foeResource resource,
                           foeResourceCreateInfo createInfo,
                           PFN_foeResourcePostLoad postLoadFn) {

    reinterpret_cast<foeShaderLoader *>(pLoader)->load(resource, createInfo, postLoadFn);
}

void foeShaderLoader::load(foeResource resource,
                           foeResourceCreateInfo createInfo,
                           PFN_foeResourcePostLoad postLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeShaderLoader - Cannot load {} as given CreateInfo is incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)),
                foeResourceCreateInfoGetType(createInfo));

        postLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                   nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    } else if (foeResourceType type = foeResourceGetType(resource);
               type != FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER &&
               type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR,
                "foeShaderLoader - Cannot load {} as it is an incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)), type);

        postLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_RESOURCE_TYPE),
                   nullptr, nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    }

    foeResultSet result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
    auto const *pShaderCI = (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
    foeShader data{
        .rType = FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER,
    };

    { // Load Shader SPIR-V from external file
        foeManagedMemory managedMemory = FOE_NULL_HANDLE;
        result = mExternalFileSearchFn(pShaderCI->pFile, &managedMemory);
        if (result.value != FOE_SUCCESS) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND);
            goto LOAD_FAILED;
        }

        uint32_t *pCode;
        uint32_t codeSize;
        foeManagedMemoryGetData(managedMemory, (void **)&pCode, &codeSize);

        result = foeGfxVkCreateShader(mGfxSession, &pShaderCI->gfxCreateInfo, codeSize, pCode,
                                      &data.shader);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }

        foeManagedMemoryDecrementUse(managedMemory);
    }

LOAD_FAILED:
    foeResourceCreateInfoDecrementRefCount(createInfo);

    if (result.value != FOE_SUCCESS) {
        // Failed at some point
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, FOE_LOG_LEVEL_ERROR, "Failed to load foeShader {}: {}",
                foeIdToString(foeResourceGetID(resource)), buffer)

        postLoadFn(resource, result, nullptr, nullptr, nullptr, nullptr);
    } else {
        // Loaded upto this point successfully
        mLoadSync.lock();
        mLoadRequests.emplace_back(LoadData{
            .resource = resource,
            .postLoadFn = postLoadFn,
            .data = std::move(data),
        });
        mLoadSync.unlock();
    }
}

void foeShaderLoader::unloadResource(void *pContext,
                                     foeResource resource,
                                     uint32_t resourceIteration,
                                     PFN_foeResourceUnloadCall unloadCallFn,
                                     bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeShaderLoader *>(pContext);

    if (immediateUnload) {
        auto unloadDataFn = [](void *pLoaderContext, void *pResourceRawData) {
            foeShader *pLoaderData = (foeShader *)pLoaderContext;
            foeShader *pResourceData = (foeShader *)pResourceRawData;

            *pLoaderData = *pResourceData;
        };

        foeShader data;

        if (unloadCallFn(resource, resourceIteration, &data, unloadDataFn)) {
            pLoader->mDestroySync.lock();
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(std::move(data));
            pLoader->mDestroySync.unlock();
        }
    } else {
        foeResourceIncrementRefCount(resource);
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .unloadCallFn = unloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}
