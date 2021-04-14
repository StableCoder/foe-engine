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

#include <foe/resource/image.hpp>

#include <foe/resource/image_loader.hpp>

#include "log.hpp"

foeImage::foeImage(foeId id, foeImageLoader *pLoader) : id{id}, pLoader{pLoader} {}

foeImage::~foeImage() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning, "foeImage {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeImage {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

foeId foeImage::getID() const noexcept { return id; }

foeResourceLoadState foeImage::getLoadState() const noexcept { return loadState; }

int foeImage::incrementRefCount() noexcept { return ++refCount; }

int foeImage::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeImage {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeImage::getRefCount() const noexcept { return refCount; }

int foeImage::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestLoad();
    }

    return newCount;
}

int foeImage::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeImage {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeImage::getUseCount() const noexcept { return useCount; }

void foeImage::requestLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}

void foeImage::requestUnload() { pLoader->requestResourceUnload(this); }