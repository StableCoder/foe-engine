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

#include <foe/resource/armature.hpp>

#include <foe/resource/armature_loader.hpp>

#include "log.hpp"

foeArmature::foeArmature(foeId id, foeArmatureLoader *pLoader) : id{id}, pLoader{pLoader} {}

foeArmature::~foeArmature() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning, "foeArmature {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeArmature {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

foeId foeArmature::getID() const noexcept { return id; }

foeResourceLoadState foeArmature::getLoadState() const noexcept { return loadState; }

int foeArmature::incrementRefCount() noexcept { return ++refCount; }

int foeArmature::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeArmature {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeArmature::getRefCount() const noexcept { return refCount; }

int foeArmature::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestLoad();
    }

    return newCount;
}

int foeArmature::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeArmature {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeArmature::getUseCount() const noexcept { return useCount; }

void foeArmature::requestLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}

void foeArmature::requestUnload() { pLoader->requestResourceUnload(this); }