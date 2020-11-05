/*
    Copyright (C) 2020 George Cave.

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

#include <foe/graphics/shader_pool.hpp>

#include "gfx_log.hpp"

VkResult foeShaderPool::initialize(VkDevice device) {
    mDevice = device;

    return VK_SUCCESS;
}

void foeShaderPool::deinitialize() {
    std::scoped_lock lock{mSync};

    for (auto *it : mShaders) {
        unload(it);
    }

    mDevice = VK_NULL_HANDLE;
}

foeShader *foeShaderPool::create(std::string_view name) {
    std::scoped_lock lock{mSync};

    for (auto *pShader : mShaders) {
        if (name == pShader->name) {
            return pShader;
        }
    }

    // Create a new entry
    foeShader *pShader = new foeShader;
    pShader->name = name;
    pShader->pManager = this;

    mShaders.emplace_back(pShader);

    return pShader;
}

foeShader *foeShaderPool::find(std::string_view name) {
    std::scoped_lock lock{mSync};

    for (auto *pShader : mShaders) {
        if (name == pShader->name) {
            return pShader;
        }
    }

    return nullptr;
}

#include "external_shader.hpp"

void foeShaderPool::load(foeShader *pShader) {
    LoadState expected = LoadState::Unloaded;
    if (!pShader->loadState.compare_exchange_strong(expected, LoadState::Loading)) {
        // Some other thread beat us to loading, leave
    }
    pShader->incrementRefCount();

    FOE_LOG(Graphics, Verbose, "Attempting to load shader: {} - {}", static_cast<void *>(pShader),
            pShader->name);

    VkShaderModule shaderModule;

    bool loaded = loadExternalShader(pShader->name, mDevice, &shaderModule);
    if (loaded) {
        pShader->module = shaderModule;

        pShader->loadState = LoadState::Loaded;
    } else {
        pShader->loadState = LoadState::Failed;
    }

    pShader->decrementRefCount();
}

void foeShaderPool::unload(foeShader *pShader) {
    auto activeUses = pShader->useCount.load();
    if (activeUses > 0) {
        FOE_LOG(Graphics, Warning, "Unloading foeShader with {} active uses: {} - {}", activeUses,
                static_cast<void *>(pShader), pShader->name);
    }

    if (pShader->module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(mDevice, pShader->module, nullptr);
    }
    pShader->module = VK_NULL_HANDLE;

    auto expected = LoadState::Loaded;
    pShader->loadState.compare_exchange_strong(expected, LoadState::Unloaded);
}