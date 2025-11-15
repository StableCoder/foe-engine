// Copyright (C) 2025 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/graphics/vk/debug_callback.h>
#include <foe/graphics/vk/result.h>
#include <foe/graphics/vk/runtime.h>

namespace {

VkBool32 vulkanMessageCallbacks(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT objectType,
                                uint64_t object,
                                size_t location,
                                int32_t messageCode,
                                char const *pLayerPrefix,
                                char const *pMessage,
                                void *pUserData) {
    return VK_FALSE;
}

} // namespace

TEST_CASE("Debug Callback (Vulkan)") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;

    // extension for Vulkan debug callback capability
    char const *extension = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

    result = foeGfxVkCreateRuntime(nullptr, 0, 0, 0, nullptr, 1, &extension, &runtime);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);

    // register the debug callback
    VkDebugReportCallbackEXT registeredCallback = VK_NULL_HANDLE;

    result = foeGfxVkRegisterDebugCallback(runtime, VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT,
                                           vulkanMessageCallbacks, &registeredCallback);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != VK_NULL_HANDLE);

    result = foeGfxVkDeregisterDebugCallback(runtime, registeredCallback);

    REQUIRE(result.value == FOE_SUCCESS);

    foeGfxDestroyRuntime(runtime);
}