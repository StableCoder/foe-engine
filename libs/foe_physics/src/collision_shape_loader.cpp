// Copyright (C) 2021-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "collision_shape_loader.hpp"

#include <foe/ecs/id_to_string.hpp>
#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_create_info.hpp>
#include <foe/physics/type_defs.h>

#include "bt_glm_conversion.hpp"
#include "log.hpp"
#include "result.h"

foeCollisionShapeLoader::~foeCollisionShapeLoader() {}

foeResultSet foeCollisionShapeLoader::initialize(foeResourcePool resourcePool) {
    if (resourcePool == FOE_NULL_HANDLE)
        return to_foeResult(FOE_PHYSICS_ERROR_COLLISION_SHAPE_LOADER_INITIALIZATION_FAILED);

    foeResultSet result = to_foeResult(FOE_PHYSICS_SUCCESS);

    mResourcePool = resourcePool;

    if (result.value != FOE_SUCCESS) {
        deinitialize();
    }

    return result;
}

void foeCollisionShapeLoader::deinitialize() {
    // Unload all resources this loader loaded
    bool upcomingWork;
    do {
        upcomingWork = foeResourcePoolUnloadType(mResourcePool,
                                                 FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE) > 0;

        maintenance();

        mLoadSync.lock();
        upcomingWork |= !mLoadRequests.empty();
        mLoadSync.unlock();

        mUnloadSync.lock();
        upcomingWork |= !mUnloadRequests.empty();
        mUnloadSync.unlock();
    } while (upcomingWork);

    // External
    mResourcePool = FOE_NULL_HANDLE;
}

bool foeCollisionShapeLoader::initialized() const noexcept { return false; }

void foeCollisionShapeLoader::maintenance() {
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
            auto *pSrcData = (foeCollisionShape *)pSrc;
            new (pDst) foeCollisionShape(std::move(*pSrcData));
        };

        if (foeResourceGetType(it.resource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
            // Replace the undefined resource with the actual resource now
            foeResource newResource = foeResourcePoolLoadedReplace(
                mResourcePool, foeResourceGetID(it.resource),
                FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE, sizeof(foeCollisionShape), &it.data,
                moveFn, this, foeCollisionShapeLoader::unloadResource);

            if (newResource == FOE_NULL_HANDLE)
                // @TODO - Handle Failure
                std::abort();

            // Decrement the references we no longer use
            foeResourceDecrementRefCount(it.resource);
            foeResourceDecrementRefCount(newResource);
        } else {
            // The resource handle is of the desired type already, use the regular load function
            it.pPostLoadFn(it.resource, to_foeResult(FOE_PHYSICS_SUCCESS), &it.data, moveFn, this,
                           foeCollisionShapeLoader::unloadResource);
        }
    }
}

bool foeCollisionShapeLoader::canProcessCreateInfo(foeResourceCreateInfo createInfo) {
    return foeResourceCreateInfoGetType(createInfo) ==
           FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_CREATE_INFO;
}

void foeCollisionShapeLoader::load(void *pLoader,
                                   foeResource resource,
                                   foeResourceCreateInfo createInfo,
                                   PFN_foeResourcePostLoad *pPostLoadFn) {
    reinterpret_cast<foeCollisionShapeLoader *>(pLoader)->load(resource, createInfo, pPostLoadFn);
}

void foeCollisionShapeLoader::load(foeResource resource,
                                   foeResourceCreateInfo createInfo,
                                   PFN_foeResourcePostLoad *pPostLoadFn) {
    if (!canProcessCreateInfo(createInfo)) {
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                "foeCollisionShapeLoader - Cannot load {} as given CreateInfo is incompatible "
                "type: {}",
                foeIdToString(foeResourceGetID(resource)),
                foeResourceCreateInfoGetType(createInfo));

        pPostLoadFn(resource, to_foeResult(FOE_PHYSICS_ERROR_INCOMPATIBLE_CREATE_INFO), nullptr,
                    nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    } else if (foeResourceType type = foeResourceGetType(resource);
               type != FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE &&
               type != FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED) {
        FOE_LOG(foePhysics, FOE_LOG_LEVEL_ERROR,
                "foeCollisionShapeLoader - Cannot load {} as it is an incompatible type: {}",
                foeIdToString(foeResourceGetID(resource)), foeResourceGetType(resource));

        pPostLoadFn(resource, to_foeResult(FOE_PHYSICS_ERROR_INCOMPATIBLE_RESOURCE_TYPE), nullptr,
                    nullptr, nullptr, nullptr);
        foeResourceCreateInfoDecrementRefCount(createInfo);
        return;
    }

    auto const *pCollisionShapeCreateInfo =
        (foeCollisionShapeCreateInfo const *)foeResourceCreateInfoGetData(createInfo);
    foeCollisionShape data{
        .rType = FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE,
    };

    data.collisionShape =
        std::make_unique<btBoxShape>(glmToBtVec3(pCollisionShapeCreateInfo->boxSize));

    foeResourceCreateInfoDecrementRefCount(createInfo);

    mLoadSync.lock();
    mLoadRequests.emplace_back(LoadData{
        .resource = resource,
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
        pLoader->mUnloadSync.lock();

        pLoader->mUnloadRequests.emplace_back(UnloadData{
            .resource = resource,
            .iteration = resourceIteration,
            .pUnloadCallFn = pUnloadCallFn,
        });

        pLoader->mUnloadSync.unlock();
    }
}