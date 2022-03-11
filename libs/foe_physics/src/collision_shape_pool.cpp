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

#include <foe/physics/resource/collision_shape_pool.hpp>

#include <foe/physics/resource/collision_shape.hpp>
#include <foe/physics/resource/collision_shape_loader.hpp>
#include <foe/physics/type_defs.h>

foeCollisionShapePool::foeCollisionShapePool(foeResourceFns const &resourceFns) :
    mResourceFns{resourceFns} {}

foeCollisionShapePool::~foeCollisionShapePool() {
    for (auto const resource : mResources) {
        foeResourceDecrementRefCount(resource);
        foeDestroyResource(resource);
    }
}

foeResource foeCollisionShapePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            return FOE_NULL_HANDLE;
        }
    }

    // Not found, add it
    foeResource newResource;
    std::error_code errC =
        foeCreateResource(resource, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE, &mResourceFns,
                          sizeof(foeCollisionShape), &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    mResources.emplace_back(newResource);

    return newResource;
}

foeResource foeCollisionShapePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            return it;
        }
    }

    // Not found, create it now
    foeResource newResource;
    std::error_code errC =
        foeCreateResource(resource, FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE, &mResourceFns,
                          sizeof(foeCollisionShape), &newResource);
    if (errC)
        return FOE_NULL_HANDLE;

    foeResourceIncrementRefCount(newResource);

    mResources.emplace_back(newResource);

    return newResource;
}

foeResource foeCollisionShapePool::find(foeResourceID resource) {
    foeResource outResource;

    mSync.lock_shared();
    for (auto const it : mResources) {
        if (foeResourceGetID(it) == resource) {
            outResource = it;
            break;
        }
    }
    mSync.unlock_shared();

    return outResource;
}

void foeCollisionShapePool::setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn) {
    mResourceFns.asyncTaskFn = asyncTaskFn;
}

void foeCollisionShapePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto const it : mResources) {
        foeResourceUnload(it, false);
    }
}