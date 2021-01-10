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

#include <foe/graphics/vk/runtime.hpp>

#include <foe/engine_detail.h>
#include <vk_error_code.hpp>

#include "debug_callback.hpp"
#include "runtime.hpp"

namespace {

void foeGfxVkDestroyRuntime(foeGfxVkRuntime const *pRuntime) {
    if (pRuntime->debugCallback != VK_NULL_HANDLE)
        foeVkDestroyDebugCallback(pRuntime->instance, pRuntime->debugCallback);

    if (pRuntime->instance != VK_NULL_HANDLE)
        vkDestroyInstance(pRuntime->instance, nullptr);

    delete pRuntime;
}

} // namespace

std::error_code foeGfxVkCreateRuntime(char const *applicationName,
                                      uint32_t applicationVersion,
                                      std::vector<std::string> layers,
                                      std::vector<std::string> extensions,
                                      bool validation,
                                      bool debugLogging,
                                      foeGfxRuntime *pRuntime) {
    auto *pNewRuntime = new foeGfxVkRuntime;

    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = applicationName,
        .applicationVersion = applicationVersion,
        .pEngineName = FOE_ENGINE_NAME,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = VK_MAKE_VERSION(1, 0, 0),
    };

    std::vector<char const *> finalLayers;
    std::vector<char const *> finalExtensions;

    if (validation)
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    if (debugLogging)
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    for (auto &it : layers)
        finalLayers.emplace_back(it.data());

    for (auto &it : extensions)
        finalExtensions.emplace_back(it.data());

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(finalLayers.size()),
        .ppEnabledLayerNames = finalLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size()),
        .ppEnabledExtensionNames = finalExtensions.data(),
    };

    VkResult vkRes = vkCreateInstance(&instanceCI, nullptr, &pNewRuntime->instance);
    if (vkRes != VK_SUCCESS)
        goto CREATE_FAILED;

    if (debugLogging) {
        vkRes = foeVkCreateDebugCallback(pNewRuntime->instance, &pNewRuntime->debugCallback);
        if (vkRes != VK_SUCCESS)
            goto CREATE_FAILED;
    }

CREATE_FAILED:
    if (vkRes != VK_SUCCESS) {
        foeGfxVkDestroyRuntime(pNewRuntime);
    } else {
        *pRuntime = reinterpret_cast<foeGfxRuntime>(pNewRuntime);
    }

    return vkRes;
}

VkInstance foeGfxVkGetInstance(foeGfxRuntime runtime) {
    auto *pRuntime = reinterpret_cast<foeGfxVkRuntime *>(runtime);
    return pRuntime->instance;
}

void foeGfxDestroyRuntime(foeGfxRuntime runtime) {
    auto *pRuntime = reinterpret_cast<foeGfxVkRuntime *>(runtime);

    foeGfxVkDestroyRuntime(pRuntime);
}