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

#include <foe/resource/fragment_descriptor.hpp>

#include <foe/resource/fragment_descriptor_loader.hpp>

#include "log.hpp"

foeFragmentDescriptor::foeFragmentDescriptor(foeFragmentDescriptorLoader *pLoader) :
    pLoader{pLoader} {}

foeFragmentDescriptor::~foeFragmentDescriptor() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeFragmentDescriptor {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeFragmentDescriptor {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

foeResourceLoadState foeFragmentDescriptor::getLoadState() const noexcept { return loadState; }

int foeFragmentDescriptor::incrementRefCount() noexcept { return ++refCount; }

int foeFragmentDescriptor::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeFragmentDescriptor {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeFragmentDescriptor::getRefCount() const noexcept { return refCount; }

int foeFragmentDescriptor::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestResourceLoad();
    }

    return newCount;
}

int foeFragmentDescriptor::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeFragmentDescriptor {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeFragmentDescriptor::getUseCount() const noexcept { return useCount; }

foeGfxVkFragmentDescriptor *foeFragmentDescriptor::getFragmentDescriptor() const noexcept {
    return data.pGfxFragDescriptor;
}

void foeFragmentDescriptor::requestResourceLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}