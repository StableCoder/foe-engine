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

#include <foe/resource/shader.hpp>

#include <foe/resource/shader_loader.hpp>

#include "log.hpp"

foeShader::foeShader(foeId id, foeShaderLoader *pLoader) : id{id}, pLoader{pLoader} {}

foeShader::~foeShader() {
    if (useCount > 0) {
        FOE_LOG(foeResource, Warning, "foeShader {} being destroyed despite having active uses",
                static_cast<void *>(this));
    }
    if (refCount > 0) {
        FOE_LOG(foeResource, Warning,
                "foeShader {} being destroyed despite having active references",
                static_cast<void *>(this));
    }
}

foeId foeShader::getID() const noexcept { return id; }

foeResourceLoadState foeShader::getLoadState() const noexcept { return loadState; }

int foeShader::incrementRefCount() noexcept { return ++refCount; }

int foeShader::decrementRefCount() noexcept {
    auto newCount = --refCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeShader {} has a negative reference count",
                static_cast<void *>(this))
    }
    return newCount;
}

int foeShader::getRefCount() const noexcept { return refCount; }

int foeShader::incrementUseCount() noexcept {
    auto newCount = ++useCount;

    // If the count was (presumably 1) and it's in the 'loading' state
    if (newCount == 1 && loadState == foeResourceLoadState::Unloaded) {
        requestLoad();
    }

    return newCount;
}

int foeShader::decrementUseCount() noexcept {
    auto newCount = --useCount;
    if (newCount < 0) {
        FOE_LOG(foeResource, Warning, "foeShader {} has a negative use count",
                static_cast<void *>(this))
    }
    return --newCount;
}

int foeShader::getUseCount() const noexcept { return useCount; }

void foeShader::requestLoad() {
    incrementRefCount();
    pLoader->requestResourceLoad(this);
}

void foeShader::requestUnload() { pLoader->requestResourceUnload(this); }

foeGfxShader foeShader::getShader() const noexcept { return data.shader; }
