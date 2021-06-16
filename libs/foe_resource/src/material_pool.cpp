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
#include <foe/resource/material_loader.hpp>

namespace {

void materialLoadFn(void *pContext, void *pResource, bool load) {
    auto *pMaterialLoader = reinterpret_cast<foeMaterialLoader *>(pContext);
    auto *pMaterial = reinterpret_cast<foeMaterial *>(pResource);

    if (load) {
        pMaterialLoader->requestResourceLoad(pMaterial);
    } else {
        pMaterialLoader->requestResourceUnload(pMaterial);
    }
}

} // namespace

foeMaterialPool::~foeMaterialPool() {
    for (auto *pMaterial : mMaterials) {
        pMaterial->decrementRefCount();

        delete pMaterial;
    }
}

foeMaterial *foeMaterialPool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pMaterial : mMaterials) {
        if (pMaterial->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeMaterial *pMaterial = new foeMaterial{resource, materialLoadFn, mpMaterialLoader};
    pMaterial->incrementRefCount();

    mMaterials.emplace_back(pMaterial);

    return pMaterial;
}

foeMaterial *foeMaterialPool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pMaterial : mMaterials) {
        if (pMaterial->getID() == resource) {
            return pMaterial;
        }
    }

    // Not found, create it now
    foeMaterial *pMaterial = new foeMaterial{resource, materialLoadFn, mpMaterialLoader};
    pMaterial->incrementRefCount();

    mMaterials.emplace_back(pMaterial);

    return pMaterial;
}

foeMaterial *foeMaterialPool::find(foeId id) {
    foeMaterial *pMaterial{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mMaterials) {
        if (pOld->getID() == id) {
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