// Copyright (C) 2021-2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/runtime.h>

#include <foe/delimited_string.h>
#include <foe/engine_detail.h>

#include "result.h"
#include "runtime.h"
#include "vk_result.h"

#include <cstring>
#include <vector>

namespace {

void foeGfxVkDestroyRuntime(foeGfxVkRuntime const *pRuntime) {
    // State
    delete[] pRuntime->pExtensionNames;
    delete[] pRuntime->pLayerNames;

    if (pRuntime->instance != VK_NULL_HANDLE)
        vkDestroyInstance(pRuntime->instance, nullptr);

    delete pRuntime;
}

} // namespace

extern "C" foeResultSet foeGfxVkCreateRuntime(char const *pApplicationName,
                                              uint32_t applicationVersion,
                                              uint32_t applicationApiVersion,
                                              uint32_t layerCount,
                                              char const *const *ppLayerNames,
                                              uint32_t extensionCount,
                                              char const *const *ppExtensionNames,
                                              foeGfxRuntime *pRuntime) {
    auto *pNewRuntime = new (std::nothrow) foeGfxVkRuntime{};
    if (pNewRuntime == nullptr) {
        return to_foeResult(FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY);
    }

    VkApplicationInfo appinfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = pApplicationName,
        .applicationVersion = applicationVersion,
        .pEngineName = FOE_ENGINE_NAME,
        .engineVersion = FOE_ENGINE_VERSION,
        .apiVersion = applicationApiVersion,
    };

    // Layers / Extensions
    std::vector<char const *> layers;
    std::vector<char const *> extensions;

    for (uint32_t i = 0; i < layerCount; ++i)
        layers.emplace_back(ppLayerNames[i]);
    for (uint32_t i = 0; i < extensionCount; ++i)
        extensions.emplace_back(ppExtensionNames[i]);

#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
    // From Vulkan Load 216+, for non-conforming implementations (such as MoltenVK),
    // need to use the portability extension
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    // Remove duplicate layers/extensions
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        for (auto cmpIt = it + 1; cmpIt != layers.end();) {
            if (strcmp(*it, *cmpIt) == 0)
                cmpIt = layers.erase(cmpIt);
            else
                ++cmpIt;
        }
    }
    for (auto it = extensions.begin(); it != extensions.end(); ++it) {
        for (auto cmpIt = it + 1; cmpIt != extensions.end();) {
            if (strcmp(*it, *cmpIt) == 0)
                cmpIt = extensions.erase(cmpIt);
            else
                ++cmpIt;
        }
    }

    // Create Instance
    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo = &appinfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    VkResult vkResult = vkCreateInstance(&instanceCI, nullptr, &pNewRuntime->instance);
    if (vkResult != VK_SUCCESS)
        goto CREATE_FAILED;

    // Se tthe runtime API version
    pNewRuntime->apiVersion = applicationApiVersion;

    // Add layer/extension state to runtime struct for future queries
    foeCreateDelimitedString(layers.size(), layers.data(), '\0', &pNewRuntime->layerNamesLength,
                             nullptr);
    if (pNewRuntime->layerNamesLength != 0) {
        pNewRuntime->pLayerNames = new char[pNewRuntime->layerNamesLength];
        foeCreateDelimitedString(layers.size(), layers.data(), '\0', &pNewRuntime->layerNamesLength,
                                 pNewRuntime->pLayerNames);
    }

    foeCreateDelimitedString(extensions.size(), extensions.data(), '\0',
                             &pNewRuntime->extensionNamesLength, nullptr);
    if (pNewRuntime->extensionNamesLength != 0) {
        pNewRuntime->pExtensionNames = new char[pNewRuntime->extensionNamesLength];
        foeCreateDelimitedString(extensions.size(), extensions.data(), '\0',
                                 &pNewRuntime->extensionNamesLength, pNewRuntime->pExtensionNames);
    }

CREATE_FAILED:
    if (vkResult != VK_SUCCESS) {
        foeGfxVkDestroyRuntime(pNewRuntime);
    } else {
        *pRuntime = runtime_to_handle(pNewRuntime);
    }

    return vk_to_foeResult(vkResult);
}

extern "C" foeResultSet foeGfxVkEnumerateRuntimeLayers(foeGfxRuntime runtime,
                                                       uint32_t *pLayerNamesLength,
                                                       char *pLayerNames) {
    auto *pRuntime = runtime_from_handle(runtime);

    return foeCopyDelimitedString(pRuntime->layerNamesLength, pRuntime->pLayerNames, '\0',
                                  pLayerNamesLength, pLayerNames)
               ? to_foeResult(FOE_GRAPHICS_VK_SUCCESS)
               : to_foeResult(FOE_GRAPHICS_VK_INCOMPLETE);
}

extern "C" foeResultSet foeGfxVkEnumerateRuntimeExtensions(foeGfxRuntime runtime,
                                                           uint32_t *pExtensionNamesLength,
                                                           char *pExtensionNames) {
    auto *pRuntime = runtime_from_handle(runtime);

    return foeCopyDelimitedString(pRuntime->extensionNamesLength, pRuntime->pExtensionNames, '\0',
                                  pExtensionNamesLength, pExtensionNames)
               ? to_foeResult(FOE_GRAPHICS_VK_SUCCESS)
               : to_foeResult(FOE_GRAPHICS_VK_INCOMPLETE);
}

extern "C" uint32_t foeGfxVkEnumerateApiVersion(foeGfxRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);

    return pRuntime->apiVersion;
}

extern "C" VkInstance foeGfxVkGetRuntimeInstance(foeGfxRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);
    return pRuntime->instance;
}

extern "C" void foeGfxDestroyRuntime(foeGfxRuntime runtime) {
    auto *pRuntime = runtime_from_handle(runtime);

    foeGfxVkDestroyRuntime(pRuntime);
}