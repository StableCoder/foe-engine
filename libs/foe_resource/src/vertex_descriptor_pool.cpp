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

#include <foe/resource/vertex_descriptor_pool.hpp>

#include <foe/resource/vertex_descriptor.hpp>

foeVertexDescriptorPool::~foeVertexDescriptorPool() {
    for (auto *pVertexDescriptor : mVertexDescriptors) {
        pVertexDescriptor->decrementRefCount();
    }
}

bool foeVertexDescriptorPool::add(foeVertexDescriptor *pVertexDescriptor) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mVertexDescriptors) {
        if (pOld->getID() == pVertexDescriptor->getID())
            return false;
    }

    pVertexDescriptor->incrementRefCount();
    mVertexDescriptors.emplace_back(pVertexDescriptor);

    return true;
}

foeVertexDescriptor *foeVertexDescriptorPool::find(foeResourceID resource) {
    foeVertexDescriptor *pVertexDescriptor{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mVertexDescriptors) {
        if (pOld->getID() == resource) {
            pVertexDescriptor = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pVertexDescriptor;
}

void foeVertexDescriptorPool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pVertexDescriptor : mVertexDescriptors) {
        pVertexDescriptor->requestUnload();
    }
}