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

#include <foe/resource/image_pool.hpp>

#include <foe/resource/image.hpp>
#include <foe/resource/image_loader.hpp>

namespace {

void imageLoadFn(void *pContext, void *pResource, bool load) {
    auto *pImageLoader = reinterpret_cast<foeImageLoader *>(pContext);
    auto *pImage = reinterpret_cast<foeImage *>(pResource);

    if (load) {
        pImageLoader->requestResourceLoad(pImage);
    } else {
        pImageLoader->requestResourceUnload(pImage);
    }
}

} // namespace

foeImagePool::~foeImagePool() {
    for (auto *pImage : mImages) {
        pImage->decrementRefCount();

        delete pImage;
    }
}

foeImage *foeImagePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pImage : mImages) {
        if (pImage->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeImage *pImage = new foeImage{resource, imageLoadFn, mpImageLoader};
    pImage->incrementRefCount();

    mImages.emplace_back(pImage);

    return pImage;
}

foeImage *foeImagePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pImage : mImages) {
        if (pImage->getID() == resource) {
            return pImage;
        }
    }

    // Not found, create it now
    foeImage *pImage = new foeImage{resource, imageLoadFn, mpImageLoader};
    pImage->incrementRefCount();

    mImages.emplace_back(pImage);

    return pImage;
}

foeImage *foeImagePool::find(foeId id) {
    foeImage *pImage{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mImages) {
        if (pOld->getID() == id) {
            pImage = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pImage;
}

void foeImagePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pImage : mImages) {
        pImage->requestUnload();
    }
}