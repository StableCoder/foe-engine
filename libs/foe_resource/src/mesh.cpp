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

#include <foe/resource/mesh.hpp>

#include <foe/resource/mesh_loader.hpp>

#include "log.hpp"

foeMesh::foeMesh(foeId id, foeMeshLoader *pLoader) : id{id}, pLoader{pLoader} {}

foeMesh::~foeMesh() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning, "foeMesh {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning, "foeMesh {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

foeId foeMesh::getID() const noexcept { return id; }

foeResourceLoadState foeMesh::getLoadState() const noexcept { return loadState; }

int foeMesh::incrementRefCount() noexcept { return ++refCount; }

int foeMesh::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeMesh {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeMesh::getRefCount() const noexcept { return refCount; }

int foeMesh::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestLoad();
    }

    return newCount;
}

int foeMesh::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeMesh {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeMesh::getUseCount() const noexcept { return useCount; }

void foeMesh::requestLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}

void foeMesh::requestUnload() { pLoader->requestResourceUnload(this); }