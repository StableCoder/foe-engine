// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/resource/shader_loader.hpp>

#include <foe/ecs/id_to_string.hpp>
#include <foe/graphics/resource/shader_create_info.hpp>
#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/shader.hpp>

#include "log.hpp"
#include "result.h"

#include <fstream>

namespace {

auto loadShaderDataFromFile(std::filesystem::path const &shaderPath) -> std::vector<std::byte> {
    std::vector<std::byte> shaderData;

    // Open file
    std::ifstream file(shaderPath, std::ifstream::binary | std::ifstream::in | std::ifstream::ate);
    if (!file.is_open())
        return shaderData;

    auto size = file.tellg();
    if (size % 4 != 0) // Must be multiple of 4
        return shaderData;

    file.seekg(0);

    shaderData.resize(size);
    file.read(reinterpret_cast<char *>(shaderData.data()), size);

    return shaderData;
}

} // namespace

foeResult foeShaderLoader::initialize(
    foeResourcePool resourcePool,
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn) {
    if (resourcePool == FOE_NULL_HANDLE || !externalFileSearchFn)
        return to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED);

    mResourcePool = resourcePool;
    mExternalFileSearchFn = externalFileSearchFn;

    return to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);
}

void foeShaderLoader::deinitialize() { mExternalFileSearchFn = {}; }

bool foeShaderLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

foeResult foeShaderLoader::initializeGraphics(foeGfxSession gfxSession) {
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
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Loads
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeShader *)pSrc;
            new (pDst) foeShader(std::move(*pSrcData));
        };

        it.pPostLoadFn(it.resource, {}, &it.data, moveFn, it.createInfo, this,
                       foeShaderLoader::unloadResource);
    }
}

bool foeShaderLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_SHADER_CREATE_INFO;
}

void foeShaderLoader::load(void *pLoader,
                           foeResource resource,
                           foeResourceCreateInfo createInfo,
                           PFN_foeResourcePostLoad *pPostLoadFn) {

    reinterpret_cast<foeShaderLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeShaderLoader::load(foeResource resource,
                           foeResourceCreateInfo createInfo,
                           PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        pPostLoadFn(resource, to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                    nullptr, nullptr, nullptr, nullptr, nullptr);
        return;
    }

    auto const *pShaderCI = (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeShader data{};
    foeResult result = to_foeResult(FOE_GRAPHICS_RESOURCE_SUCCESS);

    { // Load Shader SPIR-V from external file
        auto filePath = mExternalFileSearchFn(pShaderCI->shaderCodeFile);
        if (filePath.empty()) {
            result = to_foeResult(FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND);
            goto LOAD_FAILED;
        }

        auto shaderCode = loadShaderDataFromFile(filePath);

        result = foeGfxVkCreateShader(
            mGfxSession, &pShaderCI->gfxCreateInfo, static_cast<uint32_t>(shaderCode.size()),
            reinterpret_cast<uint32_t *>(shaderCode.data()), &data.shader);
        if (result.value != FOE_SUCCESS) {
            goto LOAD_FAILED;
        }
    }

LOAD_FAILED:
    if (result.value != FOE_SUCCESS) {
        // Failed at some point
        char buffer[FOE_MAX_RESULT_STRING_SIZE];
        result.toString(result.value, buffer);
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeShader {}: {}",
                foeIdToString(foeResourceGetID(resource)), buffer)

        pPostLoadFn(resource, result, nullptr, nullptr, nullptr, nullptr, nullptr);
    } else {
        // Loaded upto this point successfully
        mLoadSync.lock();
        mLoadRequests.emplace_back(LoadData{
            .resource = resource,
            .createInfo = createInfo,
            .pPostLoadFn = pPostLoadFn,
            .data = std::move(data),
        });
        mLoadSync.unlock();
    }
}

void foeShaderLoader::unloadResource(void *pContext,
                                     foeResource resource,
                                     uint32_t resourceIteration,
                                     PFN_foeResourceUnloadCall *pUnloadCallFn,
                                     bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeShaderLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeShader *)pSrc;
            auto *pDstData = (foeShader *)pDst;

            *pDstData = std::move(*pSrcData);
            pSrcData->~foeShader();
        };

        foeShader data{};

        if (pUnloadCallFn(resource, resourceIteration, &data, moveFn)) {
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
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}
