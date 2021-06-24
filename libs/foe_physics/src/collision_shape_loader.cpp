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

#include <foe/physics/resource/collision_shape.hpp>
#include <foe/resource/error_code.hpp>

#include "bt_glm_conversion.hpp"
#include "log.hpp"

foePhysCollisionShapeLoader::~foePhysCollisionShapeLoader() {}

std::error_code foePhysCollisionShapeLoader::initialize() {
    if (initialized()) {
        return FOE_RESOURCE_ERROR_ALREADY_INITIALIZED;
    }

    std::error_code errC{FOE_RESOURCE_SUCCESS};

INITIALIZATION_FAILED:
    if (errC) {
        deinitialize();
    }

    return errC;
}

void foePhysCollisionShapeLoader::deinitialize() {}

bool foePhysCollisionShapeLoader::initialized() const noexcept { return true; }

void foePhysCollisionShapeLoader::maintenance() {
    // Process Unloads
    mUnloadRequestsSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadRequestsSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.pCollisionShape, it.iteration, true);
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mToLoad);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        it.pCollisionShape->modifySync.lock();

        if (it.pCollisionShape->data.pUnloadFn != nullptr) {
            it.pCollisionShape->data.pUnloadFn(it.pCollisionShape->data.pUnloadContext,
                                               it.pCollisionShape, it.pCollisionShape->iteration,
                                               true);
        }

        ++it.pCollisionShape->iteration;
        it.pCollisionShape->data = std::move(it.data);
        it.pPostLoadFn(it.pCollisionShape, {});

        it.pCollisionShape->modifySync.unlock();
    }
}

bool foePhysCollisionShapeLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foePhysCollisionShapeCreateInfo *>(pCreateInfo) != nullptr;
}

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

void foePhysCollisionShapeLoader::load(
    void *pResource,
    std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
    void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pCollisionShape = reinterpret_cast<foePhysCollisionShape *>(pResource);
    auto *pCollisionShapeCreateInfo =
        reinterpret_cast<foePhysCollisionShapeCreateInfo *>(pCreateInfo.get());

    foePhysCollisionShape::Data data;

    if (!processCreateInfo(pCollisionShapeCreateInfo, data)) {
        pPostLoadFn(pResource, FOE_RESOURCE_ERROR_IMPORT_FAILED);
        return;
    }

    std::scoped_lock writeLock{pCollisionShape->modifySync};

    data.pUnloadContext = this;
    data.pUnloadFn = &foePhysCollisionShapeLoader::unloadResource;
    data.pCreateInfo = pCreateInfo;

    mLoadSync.lock();
    mToLoad.emplace_back(LoadData{
        .pCollisionShape = pCollisionShape,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foePhysCollisionShapeLoader::unloadResource(void *pContext,
                                                 void *pResource,
                                                 uint32_t resourceIteration,
                                                 bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foePhysCollisionShapeLoader *>(pContext);
    auto *pCollisionShape = reinterpret_cast<foePhysCollisionShape *>(pResource);

    if (immediateUnload) {
        pCollisionShape->modifySync.lock();

        if (pCollisionShape->iteration == resourceIteration) {
            pCollisionShape->data = {};
            ++pCollisionShape->iteration;
        }

        pCollisionShape->modifySync.unlock();
    } else {
        pLoader->mUnloadRequestsSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pCollisionShape = pCollisionShape,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadRequestsSync.unlock();
    }
}