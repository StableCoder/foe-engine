// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RUNTIME_H
#define RUNTIME_H

#include <foe/graphics/runtime.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

struct foeGfxVkRuntime {
    VkInstance instance;
    VkDebugReportCallbackEXT debugCallback;

    /// Vulkan API version the runtime was created with
    uint32_t apiVersion;
    /// Length in bytes of pLayerNames
    uint32_t layerNamesLength;
    /// Set of strings representing the instance's layers, delimited by NULL characters
    char *pLayerNames;
    /// Length in bytes of pExtensionNames
    uint32_t extensionNamesLength;
    /// Set of strings representing the instance's extensions, delimited by NULL characters
    char *pExtensionNames;
};

FOE_DEFINE_HANDLE_CASTS(runtime, foeGfxVkRuntime, foeGfxRuntime)

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_H