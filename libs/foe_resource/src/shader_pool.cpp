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

foeShaderPool::~foeShaderPool() {
    for (auto *pShader : mShaders) {
        pShader->decrementRefCount();
    }
}

bool foeShaderPool::add(foeShader *pShader) {
    std::scoped_lock lock{mSync};

    for (auto *pOld : mShaders) {
        if (pOld->getName() == pShader->getName())
            return false;
    }

    pShader->incrementRefCount();
    mShaders.emplace_back(pShader);

    return true;
}

foeShader *foeShaderPool::find(std::string_view name) {
    foeShader *pShader{nullptr};

    mSync.lock_shared();
    for (auto *pOld : mShaders) {
        if (pOld->getName() == name) {
            pShader = pOld;
            break;
        }
    }
    mSync.unlock_shared();

    return pShader;
}

foeShader *foeShaderPool::find(foeResourceID id) {
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