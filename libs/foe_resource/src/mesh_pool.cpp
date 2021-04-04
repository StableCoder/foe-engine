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

#include <foe/resource/mesh_pool.hpp>

#include <foe/resource/mesh.hpp>

foeMeshPool::~foeMeshPool() {
    for (auto *pMesh : mMeshs) {
        pMesh->decrementRefCount();
    }
}

bool foeMeshPool::add(foeMesh *pMesh) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mMeshs) {
        if (pOld->getID() == pMesh->getID())
            return false;
    }

    pMesh->incrementRefCount();
    mMeshs.emplace_back(pMesh);

    return true;
}

foeMesh *foeMeshPool::find(foeResourceID id) {
    foeMesh *pMesh{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mMeshs) {
        if (pOld->getID() == id) {
            pMesh = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pMesh;
}

void foeMeshPool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pMesh : mMeshs) {
        pMesh->requestUnload();
    }
}