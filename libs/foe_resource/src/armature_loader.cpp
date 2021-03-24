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

#include "error_code.hpp"
#include "log.hpp"

foeArmatureLoader::~foeArmatureLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(foeResource, Fatal, "foeArmatureLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foeArmatureLoader::initialize(
    std::function<
        bool(std::string_view, std::string &, std::string &, std::vector<AnimationImportInfo> &)>
        importFunction,
    std::function<void(std::function<void()>)> asynchronousJobs) {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

    mImportFunction = importFunction;
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

    std::vector<foeArmatureNode> armature;
    std::vector<foeAnimation> animations;

    std::string armatureFileName;
    std::string armatureRootNodeName;
    std::vector<AnimationImportInfo> animationImportInfo;

    // Read in the definition
    bool read = mImportFunction(pArmature->getName(), armatureFileName, armatureRootNodeName,
                                animationImportInfo);
    if (!read) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    {
        auto modelImporterPlugin = foeModelLoadFileImporterPlugin(ASSIMP_PLUGIN_PATH);

        { // Armature
            auto modelLoader = modelImporterPlugin->createImporter(armatureFileName.c_str());
            assert(modelLoader->loaded());

            auto tempArmature = modelLoader->importArmature();
            for (auto it = tempArmature.begin(); it != tempArmature.end(); ++it) {
                if (it->name == armatureRootNodeName) {
                    armature.assign(it, tempArmature.end());
                }
            }
        }

        { // Animations
            for (auto const &it : animationImportInfo) {
                auto modelLoader = modelImporterPlugin->createImporter(it.file.c_str());
                assert(modelLoader->loaded());

                for (uint32_t i = 0; i < modelLoader->getNumAnimations(); ++i) {
                    auto animName = modelLoader->getAnimationName(i);

                    for (auto const &importAnimName : it.animationNames) {
                        if (animName == importAnimName) {
                            animations.emplace_back(modelLoader->importAnimation(i));
                            break;
                        }
                    }
                }
            }
        }
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(foeResource, Error, "Failed to load foeArmature {} with error {}:{}",
                static_cast<void *>(pArmature), errC.value(), errC.message())

        pArmature->loadState = foeResourceLoadState::Failed;
    } else {
        foeArmature::Data newData{
            .armature = std::move(armature),
            .animations = std::move(animations),
        };

        // Secure the resource, and set the new data/state
        {
            std::scoped_lock writeLock{pArmature->dataWriteLock};

            //  oldData = pArmature->data;
            pArmature->data = newData;
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