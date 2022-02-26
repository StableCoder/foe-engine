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
    foeResourcePoolBase{FOE_PHYSICS_STRUCTURE_TYPE_COLLISION_SHAPE_POOL},
    mResourceFns{resourceFns} {}

foeCollisionShapePool::~foeCollisionShapePool() {
    for (auto *pCollisionShape : mCollisionShapes) {
        pCollisionShape->decrementRefCount();

        delete pCollisionShape;
    }
}

foeCollisionShape *foeCollisionShapePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pPhysCollisionShape : mCollisionShapes) {
        if (pPhysCollisionShape->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeCollisionShape *pPhysCollisionShape = new foeCollisionShape{resource, &mResourceFns};
    pPhysCollisionShape->incrementRefCount();

    mCollisionShapes.emplace_back(pPhysCollisionShape);

    return pPhysCollisionShape;
}

foeCollisionShape *foeCollisionShapePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pPhysCollisionShape : mCollisionShapes) {
        if (pPhysCollisionShape->getID() == resource) {
            return pPhysCollisionShape;
        }
    }

    // Not found, create it now
    foeCollisionShape *pPhysCollisionShape = new foeCollisionShape{resource, &mResourceFns};
    pPhysCollisionShape->incrementRefCount();

    mCollisionShapes.emplace_back(pPhysCollisionShape);

    return pPhysCollisionShape;
}

foeCollisionShape *foeCollisionShapePool::find(foeId id) {
    foeCollisionShape *pCollisionShape{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mCollisionShapes) {
        if (pOld->getID() == id) {
            pCollisionShape = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pCollisionShape;
}

void foeCollisionShapePool::setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn) {
    mResourceFns.asyncTaskFn = asyncTaskFn;
}

void foeCollisionShapePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pCollisionShape : mCollisionShapes) {
        pCollisionShape->unloadResource();
    }
}