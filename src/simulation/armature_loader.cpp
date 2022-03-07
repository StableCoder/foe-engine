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
        unloadResource(this, it.pArmature, it.iteration, true);
        it.pArmature->decrementRefCount();
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mToLoad);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        it.pArmature->modifySync.lock();

        if (it.pArmature->data.pUnloadFn != nullptr) {
            it.pArmature->data.pUnloadFn(it.pArmature->data.pUnloadContext, it.pArmature,
                                         it.pArmature->iteration, true);
        }

        ++it.pArmature->iteration;
        it.pArmature->data = std::move(it.data);
        it.pPostLoadFn(it.pArmature, {});

        it.pArmature->modifySync.unlock();
    }
}

bool foeArmatureLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeArmatureCreateInfo *>(pCreateInfo) != nullptr;
}

namespace {

bool processCreateInfo(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
    foeArmatureCreateInfo *pCreateInfo,
    foeArmature::Data &data) {
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
                             void *pResource,
                             std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                             void (*pPostLoadFn)(void *, std::error_code)) {
    reinterpret_cast<foeArmatureLoader *>(pLoader)->load(pResource, pCreateInfo, pPostLoadFn);
}

void foeArmatureLoader::load(void *pResource,
                             std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                             void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pArmature = reinterpret_cast<foeArmature *>(pResource);
    auto *pArmatureCreateInfo = dynamic_cast<foeArmatureCreateInfo *>(pCreateInfo.get());

    if (pArmatureCreateInfo == nullptr) {
        pPostLoadFn(pResource, FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO);
        return;
    }

    foeArmature::Data data{
        .pUnloadContext = this,
        .pUnloadFn = &foeArmatureLoader::unloadResource,
        .pCreateInfo = pCreateInfo,
    };

    if (!processCreateInfo(mExternalFileSearchFn, pArmatureCreateInfo, data)) {
        pPostLoadFn(pResource, FOE_BRINGUP_ERROR_IMPORT_FAILED);
        return;
    }

    std::scoped_lock lock{pArmature->modifySync};

    mLoadSync.lock();
    mToLoad.emplace_back(LoadData{
        .pArmature = pArmature,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foeArmatureLoader::unloadResource(void *pContext,
                                       void *pResource,
                                       uint32_t resourceIteration,
                                       bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeArmatureLoader *>(pContext);
    auto *pArmature = reinterpret_cast<foeArmature *>(pResource);

    if (immediateUnload) {
        pArmature->modifySync.lock();

        if (pArmature->iteration == resourceIteration) {
            pArmature->data = {};
            ++pArmature->iteration;
            pArmature->state = foeResourceState::Unloaded;
        }

        pArmature->modifySync.unlock();
    } else {
        pArmature->incrementRefCount();
        pLoader->mUnloadRequestsSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pArmature = pArmature,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadRequestsSync.unlock();
    }
}