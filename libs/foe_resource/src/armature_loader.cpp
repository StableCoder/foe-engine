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

#include <foe/resource/armature_loader.hpp>

#include <foe/resource/error_code.hpp>

#include "log.hpp"

foeArmatureLoader::~foeArmatureLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeArmatureLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeArmatureLoader::initialize(
    std::function<foeResourceCreateInfoBase *(foeId)> importFn,
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mImportFn = importFn;
    mExternalFileSearchFn = externalFileSearchFn;
    mAsyncJobs = asynchronousJobs;

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    } else {
        mInitialized = true;
    }

    return errC;
}

void foeArmatureLoader::deinitialize() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeArmatureLoader being deinitialized with {} active jobs!",
                mActiveJobs.load());
    }

    mInitialized = false;
}

bool foeArmatureLoader::initialized() const noexcept { return mInitialized; }

void foeArmatureLoader::requestResourceLoad(foeArmature *pArmature) {
    ++mActiveJobs;
    mAsyncJobs([this, pArmature] {
        loadResource(pArmature);
        --mActiveJobs;
    });
}

void foeArmatureLoader::requestResourceUnload(foeArmature *pArmature) {}

#include <foe/model/file_importer_plugins.hpp>

namespace {

bool processCreateInfo(
    std::function<std::filesystem::path(std::filesystem::path)> externalFileSearchFn,
    foeResourceCreateInfoBase *pCreateInfo,
    foeArmature::Data &data) {
    auto *createInfo = dynamic_cast<foeArmatureCreateInfo *>(pCreateInfo);
    if (createInfo == nullptr) {
        return false;
    }

    auto modelImporterPlugin = foeModelLoadFileImporterPlugin(ASSIMP_PLUGIN_PATH);

    { // Armature
        std::filesystem::path filePath = externalFileSearchFn(createInfo->fileName);
        auto modelLoader = modelImporterPlugin->createImporter(filePath.c_str());
        assert(modelLoader->loaded());

        auto tempArmature = modelLoader->importArmature();
        for (auto it = tempArmature.begin(); it != tempArmature.end(); ++it) {
            if (it->name == createInfo->rootArmatureNode) {
                data.armature.assign(it, tempArmature.end());
            }
        }
    }

    { // Animations
        for (auto const &it : createInfo->animations) {
            std::filesystem::path filePath = externalFileSearchFn(it.file);
            auto modelLoader = modelImporterPlugin->createImporter(filePath.c_str());
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

void foeArmatureLoader::loadResource(foeArmature *pArmature) {
    // First, try to enter the 'loading' state
    auto expected = pArmature->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pArmature->loadState.compare_exchange_weak(expected, foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(foeResource, Warning, "Attempted to load foeArmature {} in parrallel",
                static_cast<void *>(pArmature))
        return;
    }

    std::error_code errC;
    foeArmature::Data data;

    // Read in the definition
    std::unique_ptr<foeResourceCreateInfoBase> createInfo{mImportFn(pArmature->getID())};
    if (createInfo == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    // Process the imported definition
    if (!processCreateInfo(mExternalFileSearchFn, createInfo.get(), data)) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    } else {
        pArmature->createInfo.reset(
            reinterpret_cast<foeArmatureCreateInfo *>(createInfo.release()));
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeArmature {} with error {}:{}",
                static_cast<void *>(pArmature), errC.value(), errC.message())

        pArmature->loadState = foeResourceLoadState::Failed;
    } else {

        // Secure the resource, and set the new data/state
        {
            std::scoped_lock writeLock{pArmature->dataWriteLock};

            pArmature->data = data;
            pArmature->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        {
            // std::scoped_lock unloadLock{mUnloadSync};
            // mCurrentUnloadRequests->emplace_back(oldData);
        }
    }

    // No longer using the reference, decrement.
    pArmature->decrementRefCount();
}