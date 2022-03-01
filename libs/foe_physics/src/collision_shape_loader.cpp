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

#include <foe/physics/resource/collision_shape_loader.hpp>

#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/type_defs.h>

#include "bt_glm_conversion.hpp"
#include "error_code.hpp"
#include "log.hpp"

foeCollisionShapeLoader::foeCollisionShapeLoader() :
    foeResourceLoaderBase{FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_LOADER} {}

foeCollisionShapeLoader::~foeCollisionShapeLoader() {}

std::error_code foeCollisionShapeLoader::initialize() {
    std::error_code errC;

    if (errC) {
        deinitialize();
    }

    return errC;
}

void foeCollisionShapeLoader::deinitialize() {}

bool foeCollisionShapeLoader::initialized() const noexcept { return false; }

void foeCollisionShapeLoader::maintenance() {
    // Process Unloads
    mUnloadRequestsSync.lock();
    auto toUnload = std::move(mUnloadRequests);
    mUnloadRequestsSync.unlock();

    for (auto &it : toUnload) {
        unloadResource(this, it.pCollisionShape, it.iteration, true);
        it.pCollisionShape->decrementRefCount();
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

bool foeCollisionShapeLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeCollisionShapeCreateInfo *>(pCreateInfo) != nullptr;
}

namespace {

bool processCreateInfo(foeResourceCreateInfoBase *pCreateInfo, foeCollisionShape::Data &data) {
    auto *createInfo = dynamic_cast<foeCollisionShapeCreateInfo *>(pCreateInfo);
    if (createInfo == nullptr) {
        return false;
    }

    data.collisionShape = std::make_unique<btBoxShape>(glmToBtVec3(createInfo->boxSize));

    return true;
}

} // namespace

void foeCollisionShapeLoader::load(void *pLoader,
                                   void *pResource,
                                   std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                   void (*pPostLoadFn)(void *, std::error_code)) {
    reinterpret_cast<foeCollisionShapeLoader *>(pLoader)->load(pResource, pCreateInfo, pPostLoadFn);
}

void foeCollisionShapeLoader::load(void *pResource,
                                   std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                   void (*pPostLoadFn)(void *, std::error_code)) {
    auto *pCollisionShape = reinterpret_cast<foeCollisionShape *>(pResource);
    auto *pCollisionShapeCreateInfo =
        reinterpret_cast<foeCollisionShapeCreateInfo *>(pCreateInfo.get());

    foeCollisionShape::Data data;

    if (!processCreateInfo(pCollisionShapeCreateInfo, data)) {
        pPostLoadFn(pResource, FOE_PHYSICS_ERROR_IMPORT_FAILED);
        return;
    }

    std::scoped_lock writeLock{pCollisionShape->modifySync};

    data.pUnloadContext = this;
    data.pUnloadFn = &foeCollisionShapeLoader::unloadResource;
    data.pCreateInfo = pCreateInfo;

    mLoadSync.lock();
    mToLoad.emplace_back(LoadData{
        .pCollisionShape = pCollisionShape,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foeCollisionShapeLoader::unloadResource(void *pContext,
                                             void *pResource,
                                             uint32_t resourceIteration,
                                             bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeCollisionShapeLoader *>(pContext);
    auto *pCollisionShape = reinterpret_cast<foeCollisionShape *>(pResource);

    if (immediateUnload) {
        pCollisionShape->modifySync.lock();

        if (pCollisionShape->iteration == resourceIteration) {
            pCollisionShape->data = {};
            ++pCollisionShape->iteration;
            pCollisionShape->state = foeResourceState::Unloaded;
        }

        pCollisionShape->modifySync.unlock();
    } else {
        pCollisionShape->incrementRefCount();
        pLoader->mUnloadRequestsSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .pCollisionShape = pCollisionShape,
            .iteration = resourceIteration,
        });

        pLoader->mUnloadRequestsSync.unlock();
    }
}