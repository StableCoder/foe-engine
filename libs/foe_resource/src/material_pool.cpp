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

#include <foe/resource/material_pool.hpp>

#include <foe/resource/material.hpp>

foeMaterialPool::~foeMaterialPool() {
    for (auto *pMaterial : mMaterials) {
        pMaterial->decrementRefCount();
    }
}

bool foeMaterialPool::add(foeMaterial *pMaterial) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mMaterials) {
        if (pOld->getName() == pMaterial->getName())
            return false;
    }

    pMaterial->incrementRefCount();
    mMaterials.emplace_back(pMaterial);

    return true;
}

foeMaterial *foeMaterialPool::find(std::string_view name) {
    foeMaterial *pMaterial{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mMaterials) {
        if (pOld->getName() == name) {
            pMaterial = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pMaterial;
}

void foeMaterialPool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pMaterial : mMaterials) {
        pMaterial->requestUnload();
    }
}