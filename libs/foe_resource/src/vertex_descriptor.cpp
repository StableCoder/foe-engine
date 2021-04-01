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

#include <foe/resource/vertex_descriptor.hpp>

#include <foe/resource/shader.hpp>
#include <foe/resource/vertex_descriptor_loader.hpp>

#include "log.hpp"

foeVertexDescriptor::foeVertexDescriptor(foeResourceID id,
                                         std::string_view name,
                                         foeVertexDescriptorLoader *pLoader) :
    id{id}, name{name}, pLoader{pLoader} {}

foeVertexDescriptor::~foeVertexDescriptor() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeVertexDescriptor {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeVertexDescriptor {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

foeResourceID foeVertexDescriptor::getID() const noexcept { return id; }

std::string_view foeVertexDescriptor::getName() const noexcept { return name; }

foeResourceLoadState foeVertexDescriptor::getLoadState() const noexcept { return loadState; }

int foeVertexDescriptor::incrementRefCount() noexcept { return ++refCount; }

int foeVertexDescriptor::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeVertexDescriptor {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeVertexDescriptor::getRefCount() const noexcept { return refCount; }

int foeVertexDescriptor::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestLoad();
    }

    return newCount;
}

int foeVertexDescriptor::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeVertexDescriptor {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeVertexDescriptor::getUseCount() const noexcept { return useCount; }

void foeVertexDescriptor::requestLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}

void foeVertexDescriptor::requestUnload() { pLoader->requestResourceUnload(this); }

foeShader *foeVertexDescriptor::getVertexShader() const noexcept {
    return data.subResources.pVertex;
}

foeShader *foeVertexDescriptor::getTessellationControlShader() const noexcept {
    return data.subResources.pTessellationControl;
}

foeShader *foeVertexDescriptor::getTessellationEvaluationShader() const noexcept {
    return data.subResources.pTessellationEvaluation;
}

foeShader *foeVertexDescriptor::getGeometryShader() const noexcept {
    return data.subResources.pGeometry;
}

foeGfxVertexDescriptor const *foeVertexDescriptor::getGfxVertexDescriptor() const noexcept {
    return &data.gfxVertDescriptor;
}