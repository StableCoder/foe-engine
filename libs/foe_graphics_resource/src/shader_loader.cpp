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
    ++mDataDestroyIndex;
    if (mDataDestroyIndex >= mDataDestroyLists.size()) {
        mDataDestroyIndex = 0;
    }

    auto toDestroy = std::move(mDataDestroyLists[mDataDestroyIndex]);
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
        unloadResource(this, it.pShader, it.iteration, true);
        it.pShader->decrementRefCount();
    }

    // Loads
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        it.pShader->modifySync.lock();

        if (it.pShader->data.pUnloadFn != nullptr) {
            it.pShader->data.pUnloadFn(it.pShader->data.pUnloadContext, it.pShader,
                                       it.pShader->iteration, true);
        }

        ++it.pShader->iteration;
        it.pShader->data = std::move(it.data);
        it.pPostLoadFn(it.pShader, {});

        it.pShader->modifySync.unlock();
    }
}

bool foeShaderLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeShaderCreateInfo *>(pCreateInfo) != nullptr;
}

void foeShaderLoader::load(void *pLoader,
                           void *pResource,
                           std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                           void (*pPostLoadFn)(void *, std::error_code)) {
    reinterpret_cast<foeShaderLoader *>(pLoader)->load(pResource, pCreateInfo, pPostLoadFn);
}

void foeShaderLoader::load(void *pResource,
                           std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                           void (*pPostLoadFn)(void *, std::error_code)) {
    std::error_code errC;
    auto *pShader = reinterpret_cast<foeShader *>(pResource);
    auto *pShaderCI = dynamic_cast<foeShaderCreateInfo *>(pCreateInfo.get());

    if (pShaderCI == nullptr) {
        pPostLoadFn(pResource, FOE_GRAPHICS_RESOURCE_ERROR_INCOMPATIBLE_CREATE_INFO);
        return;
    }

    foeShader::Data shaderData{
        .pUnloadContext = this,
        .pUnloadFn = unloadResource,
        .pCreateInfo = pCreateInfo,
    };

    { // Load Shader SPIR-V from external file
        auto filePath = mExternalFileSearchFn(pShaderCI->shaderCodeFile);
        if (filePath.empty()) {
            errC = FOE_GRAPHICS_RESOURCE_ERROR_SHADER_LOADER_BINARY_FILE_NOT_FOUND;
            goto LOAD_FAILED;
        }

        auto shaderCode = loadShaderDataFromFile(filePath);

        errC = foeGfxVkCreateShader(
            mGfxSession, &pShaderCI->gfxCreateInfo, static_cast<uint32_t>(shaderCode.size()),
            reinterpret_cast<uint32_t *>(shaderCode.data()), &shaderData.shader);
        if (errC) {
            goto LOAD_FAILED;
        }
    }

LOAD_FAILED:
    if (errC) {
        // Failed at some point
        FOE_LOG(foeGraphicsResource, Error, "Failed to load foeShader {} with error {}:{}",
                foeIdToString(pShader->getID()), errC.value(), errC.message())

        pPostLoadFn(pShader, errC);
    } else {
        // Loaded upto this point successfully
        mLoadSync.lock();
        mLoadRequests.emplace_back(LoadData{
            .pShader = pShader,
            .pPostLoadFn = pPostLoadFn,
            .data = std::move(shaderData),
        });
        mLoadSync.unlock();
    }
}

void foeShaderLoader::unloadResource(void *pContext,
                                     void *pResource,
                                     uint32_t resourceIteration,
                                     bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeShaderLoader *>(pContext);
    auto *pShader = reinterpret_cast<foeShader *>(pResource);

    if (immediateUnload) {
        pShader->modifySync.lock();

        if (pShader->iteration == resourceIteration) {
            pLoader->mDataDestroyLists[pLoader->mDataDestroyIndex].emplace_back(
                std::move(pShader->data));
            pShader->data = {};
            pShader->state = foeResourceState::Unloaded;
            ++pShader->iteration;
        }

        pShader->modifySync.unlock();
    } else {
        pLoader->mUnloadRequestsSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pShader = pShader,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadRequestsSync.unlock();
    }
}
