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

foeImagePool::~foeImagePool() {
    for (auto *pImage : mImages) {
        pImage->decrementRefCount();
    }
}

bool foeImagePool::add(foeImage *pImage) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mImages) {
        if (pOld->getName() == pImage->getName())
            return false;
    }

    pImage->incrementRefCount();
    mImages.emplace_back(pImage);

    return true;
}

foeImage *foeImagePool::find(std::string_view name) {
    foeImage *pImage{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mImages) {
        if (pOld->getName() == name) {
            pImage = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pImage;
}

foeImage *foeImagePool::find(foeResourceID id) {
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