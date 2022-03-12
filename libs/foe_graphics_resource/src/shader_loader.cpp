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

#include <foe/graphics/resource/shader_loader.hpp>

#include <foe/graphics/resource/type_defs.h>
#include <foe/graphics/vk/shader.hpp>

#include "error_code.hpp"
#include "log.hpp"

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

void foeDestroyShaderCreateInfo(foeResourceCreateInfoType type, void *pCreateInfo) {
    auto *pCI = (foeShaderCreateInfo *)pCreateInfo;
    pCI->~foeShaderCreateInfo();
}

std::error_code foeShaderLoader::initialize(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn) {
    if (!externalFileSearchFn)
        return FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_INITIALIZATION_FAILED;

    mExternalFileSearchFn = externalFileSearchFn;

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

void foeShaderLoader::deinitialize() { mExternalFileSearchFn = {}; }

bool foeShaderLoader::initialized() const noexcept { return !!mExternalFileSearchFn; }

auto foeShaderLoader::initializeGraphics(foeGfxSession gfxSession) -> std::error_code {
    if (!initialized())
        return FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_NOT_INITIALIZED;

    mGfxSession = gfxSession;

    return FOE_GRAPHICS_RESOURCE_SUCCESS;
}

void foeShaderLoader::deinitializeGraphics() { mGfxSession = FOE_NULL_HANDLE; }

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
    mUnloadRequestsSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadRequestsSync.unlock();

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
        pPostLoadFn(resource, foeToErrorCode(FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO),
                    nullptr, nullptr, createInfo, nullptr, nullptr);
        return;
    }

    auto const *pShaderCI = (foeShaderCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeShader data{};
    std::error_code errC;

    { // Load Shader SPIR-V from external file
        auto filePath = mExternalFileSearchFn(pShaderCI->shaderCodeFile);
        if (filePath.empty()) {
            errC = FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND;
            goto LOAD_FAILED;
        }

        auto shaderCode = loadShaderDataFromFile(filePath);

        errC = foeGfxVkCreateShader(mGfxSession, &pShaderCI->gfxCreateInfo,
                                    static_cast<uint32_t>(shaderCode.size()),
                                    reinterpret_cast<uint32_t *>(shaderCode.data()), &data.shader);
        if (errC) {
            goto LOAD_FAILED;
        }
    }

LOAD_FAILED:
    if (errC) {
        // Failed at some point
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeShader {} with error {}:{}",
                foeIdToString(foeResourceGetID(resource)), errC.value(), errC.message())

        pPostLoadFn(resource, foeToErrorCode(errC), nullptr, nullptr, createInfo, nullptr, nullptr);
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
        pLoader->mUnloadRequestsSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadRequestsSync.unlock();
    }
}
