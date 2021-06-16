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
#include <foe/resource/vertex_descriptor_loader.hpp>

namespace {

void vertexDescriptorLoadFn(void *pContext, void *pResource, bool load) {
    auto *pVertexDescriptorLoader = reinterpret_cast<foeVertexDescriptorLoader *>(pContext);
    auto *pVertexDescriptor = reinterpret_cast<foeVertexDescriptor *>(pResource);

    if (load) {
        pVertexDescriptorLoader->requestResourceLoad(pVertexDescriptor);
    } else {
        pVertexDescriptorLoader->requestResourceUnload(pVertexDescriptor);
    }
}

} // namespace

foeVertexDescriptorPool::~foeVertexDescriptorPool() {
    for (auto *pVertexDescriptor : mVertexDescriptors) {
        pVertexDescriptor->decrementRefCount();

        delete pVertexDescriptor;
    }
}

foeVertexDescriptor *foeVertexDescriptorPool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pVertexDescriptor : mVertexDescriptors) {
        if (pVertexDescriptor->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeVertexDescriptor *pVertexDescriptor =
        new foeVertexDescriptor{resource, vertexDescriptorLoadFn, mpVertexDescriptorLoader};
    pVertexDescriptor->incrementRefCount();

    mVertexDescriptors.emplace_back(pVertexDescriptor);

    return pVertexDescriptor;
}

foeVertexDescriptor *foeVertexDescriptorPool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pVertexDescriptor : mVertexDescriptors) {
        if (pVertexDescriptor->getID() == resource) {
            return pVertexDescriptor;
        }
    }

    // Not found, create it now
    foeVertexDescriptor *pVertexDescriptor =
        new foeVertexDescriptor{resource, vertexDescriptorLoadFn, mpVertexDescriptorLoader};
    pVertexDescriptor->incrementRefCount();

    mVertexDescriptors.emplace_back(pVertexDescriptor);

    return pVertexDescriptor;
}

foeVertexDescriptor *foeVertexDescriptorPool::find(foeId resource) {
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