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
        unloadResource(this, it.resource, it.iteration, it.pUnloadCallFn, true);
        foeResourceDecrementRefCount(it.resource);
    }

    // Process Loads
    mLoadSync.lock();
    auto toLoad = std::move(mToLoad);
    mLoadSync.unlock();

    for (auto &it : toLoad) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeCollisionShape *)pSrc;
            new (pDst) foeCollisionShape(std::move(*pSrcData));
        };

        it.pPostLoadFn(it.resource, {}, &it.data, moveFn, std::move(it.pCreateInfo), this,
                       foeCollisionShapeLoader::unloadResource);
    }
}

bool foeCollisionShapeLoader::canProcessCreateInfo(foeResourceCreateInfoBase *pCreateInfo) {
    return dynamic_cast<foeCollisionShapeCreateInfo *>(pCreateInfo) != nullptr;
}

void foeCollisionShapeLoader::load(void *pLoader,
                                   foeResource resource,
                                   std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                   PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeCollisionShapeLoader *>(pLoader)->load(resource, pCreateInfo, pPostLoadFn);
}

void foeCollisionShapeLoader::load(foeResource resource,
                                   std::shared_ptr<foeResourceCreateInfoBase> const &pCreateInfo,
                                   PFN_foeResourcePostLoad *pPostLoadFn) {
    auto *pCollisionShapeCreateInfo =
        dynamic_cast<foeCollisionShapeCreateInfo *>(pCreateInfo.get());

    if (pCollisionShapeCreateInfo == nullptr) {
        pPostLoadFn(resource, foeToErrorCode(FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO), nullptr,
                    nullptr, nullptr, nullptr, nullptr);
        return;
    }

    foeCollisionShape data{};

    data.collisionShape =
        std::make_unique<btBoxShape>(glmToBtVec3(pCollisionShapeCreateInfo->boxSize));

    mLoadSync.lock();
    mToLoad.emplace_back(LoadData{
        .resource = resource,
        .pCreateInfo = pCreateInfo,
        .pPostLoadFn = pPostLoadFn,
        .data = std::move(data),
    });
    mLoadSync.unlock();
}

void foeCollisionShapeLoader::unloadResource(void *pContext,
                                             foeResource resource,
                                             uint32_t resourceIteration,
                                             PFN_foeResourceUnloadCall *pUnloadCallFn,
                                             bool immediateUnload) {
    auto *pLoader = reinterpret_cast<foeCollisionShapeLoader *>(pContext);

    if (immediateUnload) {
        auto moveFn = [](void *pSrc, void *pDst) {
            auto *pSrcData = (foeCollisionShape *)pSrc;
            auto *pDstData = (foeCollisionShape *)pDst;

            *pDstData = std::move(*pSrcData);
            pSrcData->~foeCollisionShape();
        };

        foeCollisionShape data{};

        pUnloadCallFn(resource, resourceIteration, &data, moveFn);
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