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

#include <foe/resource/shader_pool.hpp>

#include <foe/resource/shader.hpp>
#include <foe/resource/shader_loader.hpp>

namespace {

void shaderLoadFn(void *pContext, void *pResource, bool load) {
    auto *pShaderLoader = reinterpret_cast<foeShaderLoader *>(pContext);
    auto *pShader = reinterpret_cast<foeShader *>(pResource);

    if (load) {
        pShaderLoader->requestResourceLoad(pShader);
    } else {
        pShaderLoader->requestResourceUnload(pShader);
    }
}

} // namespace

foeShaderPool::~foeShaderPool() {
    for (auto *pShader : mShaders) {
        pShader->decrementRefCount();

        delete pShader;
    }
}

foeShader *foeShaderPool::add(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    // If it finds it, return nullptr
    for (auto *pShader : mShaders) {
        if (pShader->getID() == resource) {
            return nullptr;
        }
    }

    // Not found, add it
    foeShader *pShader = new foeShader{resource, shaderLoadFn, mpShaderLoader};
    pShader->incrementRefCount();

    mShaders.emplace_back(pShader);

    return pShader;
}

foeShader *foeShaderPool::findOrAdd(foeResourceID resource) {
    std::scoped_lock lock{mSync};

    for (auto *pShader : mShaders) {
        if (pShader->getID() == resource) {
            return pShader;
        }
    }

    // Not found, create it now
    foeShader *pShader = new foeShader{resource, shaderLoadFn, mpShaderLoader};
    pShader->incrementRefCount();

    mShaders.emplace_back(pShader);

    return pShader;
}

foeShader *foeShaderPool::find(foeId id) {
    foeShader *pShader{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mShaders) {
        if (pOld->getID() == id) {
            pShader = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pShader;
}

void foeShaderPool::unloadAll() {
    std::scoped_lock lock{mSync};

    for (auto *pShader : mShaders) {
        pShader->requestUnload();
    }
}