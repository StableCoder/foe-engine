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

#include <foe/physics/resource/collision_shape_pool.hpp>

#include <foe/physics/resource/collision_shape.hpp>

foePhysCollisionShapePool::~foePhysCollisionShapePool() {
    for (auto *pCollisionShape : mCollisionShapes) {
        pCollisionShape->decrementRefCount();

        delete pCollisionShape;
    }
}

bool foePhysCollisionShapePool::add(foePhysCollisionShape *pCollisionShape) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mCollisionShapes) {
        if (pOld->getID() == pCollisionShape->getID())
            return false;
    }

    pCollisionShape->incrementRefCount();
    mCollisionShapes.emplace_back(pCollisionShape);

    return true;
}

foePhysCollisionShape *foePhysCollisionShapePool::find(foeId id) {
    foePhysCollisionShape *pCollisionShape{nullptr};

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

void foePhysCollisionShapePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pCollisionShape : mCollisionShapes) {
        pCollisionShape->requestUnload();
    }
}