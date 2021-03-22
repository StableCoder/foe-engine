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

foeArmaturePool::~foeArmaturePool() {
    for (auto *pArmature : mArmatures) {
        pArmature->decrementRefCount();
    }
}

bool foeArmaturePool::add(foeArmature *pArmature) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mArmatures) {
        if (pOld->getName() == pArmature->getName())
            return false;
    }

    pArmature->incrementRefCount();
    mArmatures.emplace_back(pArmature);

    return true;
}

foeArmature *foeArmaturePool::find(std::string_view name) {
    foeArmature *pArmature{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mArmatures) {
        if (pOld->getName() == name) {
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