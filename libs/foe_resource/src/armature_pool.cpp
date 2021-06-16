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

#include <foe/resource/armature_pool.hpp>

#include <foe/resource/armature.hpp>
#include <foe/resource/armature_loader.hpp>

namespace {

void armatureLoadFn(void *pContext, void *pResource, bool load) {
    auto *pArmatureLoader = reinterpret_cast<foeArmatureLoader *>(pContext);
    auto *pArmature = reinterpret_cast<foeArmature *>(pResource);

    if (load) {
        pArmatureLoader->requestResourceLoad(pArmature);
    } else {
        pArmatureLoader->requestResourceUnload(pArmature);
    }
}

} // namespace

foeArmaturePool::~foeArmaturePool() {
    for (auto *pArmature : mArmatures) {
        pArmature->decrementRefCount();

        delete pArmature;
    }
}

foeArmature *foeArmaturePool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pArmature : mArmatures) {
        if (pArmature->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeArmature *pArmature = new foeArmature{resource, armatureLoadFn, mpArmatureLoader};
    pArmature->incrementRefCount();

    mArmatures.emplace_back(pArmature);

    return pArmature;
}

foeArmature *foeArmaturePool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pArmature : mArmatures) {
        if (pArmature->getID() == resource) {
            return pArmature;
        }
    }

    // Not found, create it now
    foeArmature *pArmature = new foeArmature{resource, armatureLoadFn, mpArmatureLoader};
    pArmature->incrementRefCount();

    mArmatures.emplace_back(pArmature);

    return pArmature;
}

foeArmature *foeArmaturePool::find(foeId id) {
    foeArmature *pArmature{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mArmatures) {
        if (pOld->getID() == id) {
            pArmature = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pArmature;
}

void foeArmaturePool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pArmature : mArmatures) {
        pArmature->requestUnload();
    }
}