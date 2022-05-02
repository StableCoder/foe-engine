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

#include "armature_loader.hpp"

#include <foe/model/assimp/importer.hpp>

#include "../error_code.hpp"
#include "../log.hpp"
#include "armature.hpp"
#include "armature_create_info.hpp"
#include "type_defs.h"

std::error_code foeArmatureLoader::initialize(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn) {
    std::error_code errC;

    mExternalFileSearchFn = externalFileSearchFn;

    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeArmatureLoader::deinitialize() { mExternalFileSearchFn = {}; }

bool foeArmatureLoader::initialized() const noexcept { return bool(mExternalFileSearchFn); }

void foeArmatureLoader::maintenance() {
    // Process Unloads
    mUnloadRequestsSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadRequestsSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mToLoad);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeArmature *)pSrc;
            new (pDst) foeArmature(std::move(*pSrcData));
        };

        it.pPostLoadFn(it.resource, {}, &it.data, moveFn, it.createInfo, this,
                       foeArmatureLoader::unloadResource);
    }
}

bool foeArmatureLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE_CREATE_INFO;
}

namespace {

bool processCreateInfo(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
    foeArmatureCreateInfo const *pCreateInfo,
    foeArmature &data) {
    { // Armature
        std::filesystem::path filePath = externalFileSearchFn(pCreateInfo->fileName);
        auto modelLoader = std::make_unique<foeModelAssimpImporter>(filePath.string().c_str(), 0);
        assert(modelLoader->loaded());

        auto tempArmature = modelLoader->importArmature();
        for (auto it = tempArmature.begin(); it != tempArmature.end(); ++it) {
            if (it->name == pCreateInfo->rootArmatureNode) {
                data.armature.assign(it, tempArmature.end());
            }
        }
    }

    { // Animations
        for (auto const &it : pCreateInfo->animations) {
            std::filesystem::path filePath = externalFileSearchFn(it.file);
            auto modelLoader =
                std::make_unique<foeModelAssimpImporter>(filePath.string().c_str(), 0);
            assert(modelLoader->loaded());

            for (uint32_t i = 0; i < modelLoader->getNumAnimations(); ++i) {
                auto animName = modelLoader->getAnimationName(i);

                for (auto const &importAnimName : it.animationNames) {
                    if (animName == importAnimName) {
                        data.animations.emplace_back(modelLoader->importAnimation(i));
                        break;
                    }
                }
            }
        }
    }

    return true;
}

} // namespace

void foeArmatureLoader::load(void *pLoader,
                             foeResource resource,
                             foeResourceCreateInfo createInfo,
                             PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeArmatureLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeArmatureLoader::load(foeResource resource,
                             foeResourceCreateInfo createInfo,
                             PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        pPostLoadFn(resource, foeToErrorCode(FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO), nullptr,
                    nullptr, nullptr, nullptr, nullptr);
        return;
    }

    auto const *pArmatureCreateInfo =
        (foeArmatureCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeArmature data{};

    if (!processCreateInfo(mExternalFileSearchFn, pArmatureCreateInfo, data)) {
        pPostLoadFn(resource, foeToErrorCode(FOE_BRINGUP_ERROR_IMPORT_FAILED), nullptr, nullptr,
                    nullptr, nullptr, nullptr);
        return;
    }

    mLoadSync.lock();
    mToLoad.emplace_back(LoadData{
        .resource = resource,
        .createInfo = createInfo,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foeArmatureLoader::unloadResource(void *pContext,
                                       foeResource resource,
                                       uint32_t resourceIteration,
                                       PFN_foeResourceUnloadCall *pUnloadCallFn,
                                       bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeArmatureLoader *>(pContext);

    if (immediateUnload) {
        auto destroyFn = [](void *pSrc, void *pDst) {
            foeArmature *pArmature = (foeArmature *)pSrc;
            pArmature->~foeArmature();
        };

        pUnloadCallFn(resource, resourceIteration, nullptr, destroyFn);
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