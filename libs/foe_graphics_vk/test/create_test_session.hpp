// Copyright (C) 2022-2023 George Cave
//
// SPDX-License-Identifier: Apache-2.0

#ifndef CREATE_TEST_SESSION_HPP
#define CREATE_TEST_SESSION_HPP

#include <catch2/catch_test_macros.hpp>
#include <foe/graphics/vk/result.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>
#include <foe/result.h>

#include "custom_main.hpp"

#include <memory>

inline foeResultSet createTestSession(foeGfxRuntime *pRuntime, foeGfxSession *pSession) {
    VkResult vkResult;
    foeResultSet result;
    uint32_t vkApiVersion;
    uint32_t vkPhysicalDeviceCount;
    std::unique_ptr<VkPhysicalDevice[]> vkPhysicalDevices;

    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;

    vkEnumerateInstanceVersion(&vkApiVersion);

    result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, false, false,
                                   &runtime);
    if (result.value != FOE_SUCCESS)
        goto CREATE_TEST_SESSION_FAILED;

    vkResult = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                          &vkPhysicalDeviceCount, nullptr);

    vkPhysicalDevices.reset(new VkPhysicalDevice[vkPhysicalDeviceCount]);

    vkResult = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                          &vkPhysicalDeviceCount, vkPhysicalDevices.get());
    if (vkResult != VK_SUCCESS && vkResult != VK_INCOMPLETE) {
        result.value = FOE_GRAPHICS_VK_ERROR_OUT_OF_MEMORY;
        goto CREATE_TEST_SESSION_FAILED;
    }

    result = foeGfxVkCreateSession(runtime, vkPhysicalDevices[getGpuSelection()], 0, nullptr, 0,
                                   nullptr, nullptr, nullptr, &session);
    if (result.value != FOE_SUCCESS)
        goto CREATE_TEST_SESSION_FAILED;

CREATE_TEST_SESSION_FAILED:
    if (result.value != FOE_SUCCESS) {
        if (session != FOE_NULL_HANDLE)
            foeGfxDestroySession(session);

        if (runtime != FOE_NULL_HANDLE)
            foeGfxDestroyRuntime(runtime);
    } else {
        *pRuntime = runtime;
        *pSession = session;
    }

    return result;
}

#endif // CREATE_TEST_SESSION_HPP