// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "armature_loader.hpp"

#include <foe/model/assimp/importer.hpp>

#include "../log.hpp"
#include "../result.h"
#include "armature.hpp"
#include "armature_create_info.hpp"
#include "type_defs.h"

foeResultSet foeArmatureLoader::initialize(
    foeResourcePool resourcePool,
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn) {
    if (resourcePool == FOE_NULL_HANDLE || !externalFileSearchFn)
        return to_foeResult(FOE_BRINGUP_ERROR_LOADER_INITIALIZATION_FAILED);

    foeResultSet result{.value = FOE_SUCCESS, .toString = NULL};

    mResourcePool = resourcePool;
    mExternalFileSearchFn = externalFileSearchFn;

    if (result.value != FOE_SUCCESS) {
        deinitialize();
    }

    return result;
}

void foeArmatureLoader::deinitialize() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork =
            foeResourcePoolUnloadType(mResourcePool, FOE_BRINGUP_STRUCTURE_TYPE_ARMATURE) > 0;

        maintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();
    } while (upcomingWork);

    // External
    mExternalFileSearchFn = {};
    mResourcePool = FOE_NULL_HANDLE;
}

bool foeArmatureLoader::initialized() const noexcept { return bool(mExternalFileSearchFn); }

void foeArmatureLoader::maintenance() {
    // Process Unloads
    mUnloadSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mLoadRequests);
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
        for (uint32_t i = 0; i < pCreateInfo->animationCount; ++i) {
            auto const &animation = pCreateInfo->pAnimations[i];
            std::filesystem::path filePath = externalFileSearchFn(animation.file);
            auto modelLoader =
                std::make_unique<foeModelAssimpImporter>(filePath.string().c_str(), 0);
            assert(modelLoader->loaded());

            for (uint32_t i = 0; i < modelLoader->getNumAnimations(); ++i) {
                auto animName = modelLoader->getAnimationName(i);

                if (animName == animation.animationName) {
                    data.animations.emplace_back(modelLoader->importAnimation(i));
                    break;
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
        pPostLoadFn(resource, to_foeResult(FOE_BRINGUP_ERROR_INCOMPATIBLE_CREATE_INFO), nullptr,
                    nullptr, nullptr, nullptr, nullptr);
        return;
    }

    auto const *pArmatureCreateInfo =
        (foeArmatureCreateInfo const *)foeResourceCreateInfoGetData(createInfo);

    foeArmature data{};

    if (!processCreateInfo(mExternalFileSearchFn, pArmatureCreateInfo, data)) {
        pPostLoadFn(resource, to_foeResult(FOE_BRINGUP_ERROR_IMPORT_FAILED), nullptr, nullptr,
                    nullptr, nullptr, nullptr);
        return;
    }

    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
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
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}