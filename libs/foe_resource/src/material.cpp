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

#include <foe/resource/material.hpp>

#include <foe/resource/fragment_descriptor.hpp>
#include <foe/resource/material_loader.hpp>

#include "log.hpp"

foeMaterial::foeMaterial(std::string_view name, foeMaterialLoader *pLoader) :
    name{name}, pLoader{pLoader} {}

foeMaterial::~foeMaterial() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning, "foeMaterial {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeMaterial {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

std::string_view foeMaterial::getName() const noexcept { return name; }

foeResourceLoadState foeMaterial::getLoadState() const noexcept { return loadState; }

int foeMaterial::incrementRefCount() noexcept { return ++refCount; }

int foeMaterial::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeMaterial {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeMaterial::getRefCount() const noexcept { return refCount; }

int foeMaterial::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestLoad();
    }

    return newCount;
}

int foeMaterial::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeMaterial {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeMaterial::getUseCount() const noexcept { return useCount; }

void foeMaterial::requestLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}

void foeMaterial::requestUnload() { pLoader->requestResourceUnload(this); }

foeFragmentDescriptor *foeMaterial::getFragmentDescriptor() const noexcept {
    return data.subResources.pFragmentDescriptor;
}

foeGfxVkFragmentDescriptor *foeMaterial::getGfxFragmentDescriptor() const noexcept {
    if (data.subResources.pFragmentDescriptor != nullptr) {
        return data.subResources.pFragmentDescriptor->getFragmentDescriptor();
    }

    return nullptr;
}

VkDescriptorSet foeMaterial::getVkDescriptorSet(uint32_t frameIndex) {
    return pLoader->createDescriptorSet(this, frameIndex);
}