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
#include "delimited_strings.hpp"
#include "log.hpp"
#include "runtime.hpp"

namespace {

void foeGfxVkDestroyRuntime(foeGfxVkRuntime const *pRuntime) {
    // State
    delete[] pRuntime->pExtensions;
    delete[] pRuntime->pLayers;

    if (pRuntime->debugCallback != VK_NULL_HANDLE)
        foeVkDestroyDebugCallback(pRuntime->instance, pRuntime->debugCallback);

    if (pRuntime->instance != VK_NULL_HANDLE)
        vkDestroyInstance(pRuntime->instance, nullptr);

    delete pRuntime;
}

} // namespace

std::error_code foeGfxVkCreateRuntime(char const *pApplicationName,
                                      uint32_t applicationVersion,
                                      std::vector<std::string> layers,
                                      std::vector<std::string> extensions,
                                      bool validation,
                                      bool debugLogging,
                                      foeGfxRuntime *pRuntime) {
    auto *pNewRuntime = new foeGfxVkRuntime;
    *pNewRuntime = {};

    // Always use the latest available runtime
    uint32_t vkApiVersion;
    vkEnumerateInstanceVersion(&vkApiVersion);

    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = pApplicationName,
        .applicationVersion = applicationVersion,
        .pEngineName = FOE_ENGINE_NAME,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = vkApiVersion,
    };

    std::vector<char const *> finalLayers;
    std::vector<char const *> finalExtensions;

    if (validation) {
        layers.emplace_back("VK_LAYER_KHRONOS_validation");
        FOE_LOG(foeVkGraphics, Verbose, "Adding validation layers to new VkInstance");
    }
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

    // Add layer/extension state to runtime struct for future queries
    convertToDelimitedString(finalExtensions.size(), finalExtensions.data(),
                             &pNewRuntime->layersLength, &pNewRuntime->pLayers);
    convertToDelimitedString(finalExtensions.size(), finalExtensions.data(),
                             &pNewRuntime->extensionsLength, &pNewRuntime->pExtensions);

    if (debugLogging) {
        vkRes = foeVkCreateDebugCallback(pNewRuntime->instance, &pNewRuntime->debugCallback);
        if (vkRes != VK_SUCCESS)
            goto CREATE_FAILED;

        FOE_LOG(foeVkGraphics, Verbose, "Added debug logging to new VkInstance");
    }

CREATE_FAILED:
    if (vkRes != VK_SUCCESS) {
        foeGfxVkDestroyRuntime(pNewRuntime);
    } else {
        *pRuntime = runtime_to_handle(pNewRuntime);
    }

    return vkRes;
}

std::error_code foeGfxVkEnumerateRuntimeLayers(foeGfxRuntime runtime,
                                               uint32_t *pLayerNamesLength,
                                               char *pLayerNames) {
    auto *pRuntime = runtime_from_handle(runtime);

    return getDelimitedString(pRuntime->layersLength, pRuntime->pLayers, pLayerNamesLength,
                              pLayerNames);
}

std::error_code foeGfxVkEnumerateRuntimeExtensions(foeGfxRuntime runtime,
                                                   uint32_t *pExtensionNamesLength,
                                                   char *pExtensionNames) {
    auto *pRuntime = runtime_from_handle(runtime);

    return getDelimitedString(pRuntime->extensionsLength, pRuntime->pExtensions,
                              pExtensionNamesLength, pExtensionNames);
}

VkInstance foeGfxVkGetInstance(foeGfxRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    return pRuntime->instance;
}

void foeGfxDestroyRuntime(foeGfxRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);

    foeGfxVkDestroyRuntime(pRuntime);
}