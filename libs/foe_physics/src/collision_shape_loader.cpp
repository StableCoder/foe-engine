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

#include <foe/physics/resource/collision_shape_loader.hpp>

#include <foe/log.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/resource/error_code.hpp>

#include "bt_glm_conversion.hpp"

foePhysCollisionShapeLoader::~foePhysCollisionShapeLoader() {
    if (mActiveJobs > 0) {
        FOE_LOG(General, Fatal, "foePhysCollisionShapeLoader being destructed with {} active jobs!",
                mActiveJobs.load());
    }
}

std::error_code foePhysCollisionShapeLoader::initialize(
    std::function<foeResourceCreateInfoBase *(foeResourceID)> importFunction,
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

void foePhysCollisionShapeLoader::deinitialize() {
    if (mActiveJobs > 0) {
        FOE_LOG(General, Fatal,
                "foePhysCollisionShapeLoader being deinitialized with {} active jobs!",
                mActiveJobs.load());
    }

    mInitialized = false;
}

bool foePhysCollisionShapeLoader::initialized() const noexcept { return mInitialized; }

void foePhysCollisionShapeLoader::requestResourceLoad(foePhysCollisionShape *pCollisionShape) {
    ++mActiveJobs;
    mAsyncJobs([this, pCollisionShape] {
        loadResource(pCollisionShape);
        --mActiveJobs;
    });
}

void foePhysCollisionShapeLoader::requestResourceUnload(foePhysCollisionShape *pCollisionShape) {}

namespace {

bool processCreateInfo(foeResourceCreateInfoBase *pCreateInfo, foePhysCollisionShape::Data &data) {
    auto *createInfo = dynamic_cast<foePhysCollisionShapeCreateInfo *>(pCreateInfo);
    if (createInfo == nullptr) {
        return false;
    }

    data.collisionShape = std::make_unique<btBoxShape>(glmToBtVec3(createInfo->boxSize));

    return true;
}

} // namespace

void foePhysCollisionShapeLoader::loadResource(foePhysCollisionShape *pCollisionShape) {
    FOE_LOG(General, Warning, "Attempted to load foePhysCollisionShape {}",
            static_cast<void *>(pCollisionShape))
    // First, try to enter the 'loading' state
    auto expected = pCollisionShape->loadState.load();
    while (expected != foeResourceLoadState::Loading) {
        if (pCollisionShape->loadState.compare_exchange_weak(expected,
                                                             foeResourceLoadState::Loading))
            break;
    }
    if (expected == foeResourceLoadState::Loading) {
        FOE_LOG(General, Warning, "Attempted to load foePhysCollisionShape {} in parrallel",
                static_cast<void *>(pCollisionShape))
        return;
    }

    std::error_code errC;
    foePhysCollisionShape::Data data;

    // Read in the definition
    std::unique_ptr<foeResourceCreateInfoBase> createInfo{
        mImportFunction(pCollisionShape->getID())};
    if (createInfo == nullptr) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    }

    // Process the imported definition
    if (!processCreateInfo(createInfo.get(), data)) {
        errC = FOE_RESOURCE_ERROR_IMPORT_FAILED;
        goto LOADING_FAILED;
    } else {
        pCollisionShape->createInfo.reset(
            reinterpret_cast<foePhysCollisionShapeCreateInfo *>(createInfo.release()));
    }

LOADING_FAILED:
    if (errC) {
        FOE_LOG(General, Error, "Failed to load foePhysCollisionShape {} with error {}:{}",
                static_cast<void *>(pCollisionShape), errC.value(), errC.message())

        pCollisionShape->loadState = foeResourceLoadState::Failed;
    } else {

        // Secure the resource, and set the new data/state
        {
            std::scoped_lock writeLock{pCollisionShape->dataWriteLock};

            pCollisionShape->data = std::move(data);
            pCollisionShape->loadState = foeResourceLoadState::Loaded;
        }

        // If there was active old data that we just wrote over, send it to be unloaded
        {
            // std::scoped_lock unloadLock{mUnloadSync};
            // mCurrentUnloadRequests->emplace_back(oldData);
        }
    }

    // No longer using the reference, decrement.
    pCollisionShape->decrementRefCount();
}