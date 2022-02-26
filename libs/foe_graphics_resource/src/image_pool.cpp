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

#include <foe/graphics/resource/image_pool.hpp>

#include <foe/graphics/resource/image.hpp>
#include <foe/graphics/resource/type_defs.h>

foeImagePool::foeImagePool(foeResourceFns const &resourceFns) :
    foeResourcePoolBase{FOE_GRAPHICS_RESOURCE_STRUCTURE_TYPE_IMAGE_POOL},
    mResourceFns{resourceFns} {}

foeImagePool::~foeImagePool() {
    for (auto *pResource : mResources) {
        pResource->decrementRefCount();

        delete pResource;
    }
}

foeImage *foeImagePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pResource : mResources) {
        if (pResource->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeImage *pResource = new foeImage{resource, &mResourceFns};
    pResource->incrementRefCount();

    mResources.emplace_back(pResource);

    return pResource;
}

foeImage *foeImagePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pResource : mResources) {
        if (pResource->getID() == resource) {
            return pResource;
        }
    }

    // Not found, create it now
    foeImage *pResource = new foeImage{resource, &mResourceFns};
    pResource->incrementRefCount();

    mResources.emplace_back(pResource);

    return pResource;
}

foeImage *foeImagePool::find(foeId id) {
    foeImage *pResource{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mResources) {
        if (pOld->getID() == id) {
            pResource = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pResource;
}

void foeImagePool::setAsyncTaskFn(std::function<void(std::function<void()>)> asyncTaskFn) {
    mResourceFns.asyncTaskFn = asyncTaskFn;
}

void foeImagePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pResource : mResources) {
        pResource->unloadResource();
    }
}