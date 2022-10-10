// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/graphics/type_defs.h>
#include <foe/graphics/vk/result.h>
#include <foe/graphics/vk/runtime.h>
#include <foe/graphics/vk/session.h>

#include <string.h>

TEST_CASE("foeGfxSession(Vulkan)") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;
    uint32_t vkApiVersion;
    uint32_t physicalDeviceCount = 1;
    VkPhysicalDevice vkPhysicalDevice = FOE_NULL_HANDLE;

    uint32_t strLen;
    char strBuffer[512];

    vkEnumerateInstanceVersion(&vkApiVersion);

    result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, false, false,
                                   &runtime);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);

    VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                &physicalDeviceCount, &vkPhysicalDevice);
    REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
    REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

    SECTION("No layers or extensions") {
        result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, nullptr,
                                       nullptr, &session);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(session != FOE_NULL_HANDLE);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateSessionLayers(session, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 0);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateSessionExtensions(session, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 0);
    }
    SECTION("With an extension") {
        char extension[] = "VK_KHR_swapchain";
        char *ppExtensions = extension;

        result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 1, &ppExtensions,
                                       nullptr, nullptr, &session);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(session != FOE_NULL_HANDLE);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateSessionLayers(session, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 0);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateSessionExtensions(session, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 17);

        SECTION("Enumerating Extensions") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateSessionExtensions(session, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 17);
                CHECK(std::string_view{strBuffer} == "VK_KHR_swapchain");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateSessionExtensions(session, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 17);
                CHECK(std::string_view{strBuffer} == "VK_KHR_swapchain");
            }
            SECTION("Undersized buffer provided") {
                strLen = 10;
                result = foeGfxVkEnumerateSessionExtensions(session, &strLen, strBuffer);
                CHECK(result.value == FOE_GRAPHICS_VK_INCOMPLETE);
                CHECK(strLen == 0);
            }
        }
    }

    // Check Vulkan handles exist
    CHECK(foeGfxVkGetInstance(session) != VK_NULL_HANDLE);
    CHECK(foeGfxVkGetPhysicalDevice(session) != VK_NULL_HANDLE);
    CHECK(foeGfxVkGetDevice(session) != VK_NULL_HANDLE);
    CHECK(foeGfxVkGetAllocator(session) != VK_NULL_HANDLE);

    // Check that real queues are returned
    CHECK(getFirstQueue(session) != nullptr);

    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_GRAPHICS_BIT) < MaxQueuesPerFamily);
    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_COMPUTE_BIT) < MaxQueuesPerFamily);
    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_TRANSFER_BIT) < MaxQueuesPerFamily);

    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT) <
          MaxQueuesPerFamily);
    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT) <
          MaxQueuesPerFamily);
    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) <
          MaxQueuesPerFamily);

    CHECK(foeGfxVkGetBestQueue(session, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
                                            VK_QUEUE_TRANSFER_BIT) < MaxQueuesPerFamily);

    // Builtin descriptor sets
    CHECK(foeGfxVkGetDummySet(session) != VK_NULL_HANDLE);

    CHECK(foeGfxVkGetBuiltinLayout(
              session, FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_PROJECTION_VIEW_MATRIX) != VK_NULL_HANDLE);

    CHECK(foeGfxVkGetBuiltinLayout(session, FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_MODEL_MATRIX) !=
          VK_NULL_HANDLE);

    CHECK(foeGfxVkGetBuiltinLayout(
              session, FOE_BUILTIN_DESCRIPTOR_SET_LAYOUT_BONE_STATE_MATRICES) != VK_NULL_HANDLE);

    // Graphics Pools exist and are returned
    CHECK(foeGfxVkGetRenderPassPool(session) != nullptr);
    CHECK(foeGfxVkGetFragmentDescriptorPool(session) != nullptr);

    // Wait for idle
    foeGfxWaitIdle(session);

    // With no requested features, no features should be returned"
    VkPhysicalDeviceFeatures basicFeatures = {};
    VkPhysicalDeviceFeatures basicFeaturesCmp = {};

#ifdef VK_KHR_get_physical_device_properties2
    VkPhysicalDeviceFeatures2 features_1_0 = {.sType =
                                                  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2},
                              features_1_0_cmp = features_1_0;
#endif
#ifdef VK_VERSION_1_1
    VkPhysicalDeviceVulkan11Features
        features_1_1 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES},
        features_1_1_cmp = features_1_1;
    features_1_0.pNext = &features_1_1;
    features_1_0_cmp.pNext = &features_1_1;
#endif
#ifdef VK_VERSION_1_2
    VkPhysicalDeviceVulkan12Features
        features_1_2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES},
        features_1_2_cmp = features_1_2;
    features_1_1.pNext = &features_1_2;
    features_1_1_cmp.pNext = &features_1_2;
#endif
#ifdef VK_VERSION_1_3
    VkPhysicalDeviceVulkan13Features
        features_1_3 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES},
        features_1_3_cmp = features_1_3;
    features_1_2.pNext = &features_1_3;
    features_1_2_cmp.pNext = &features_1_3;
#endif

#ifdef VK_KHR_get_physical_device_properties2
    foeGfxVkEnumerateSessionFeatures(session, &basicFeatures, &features_1_0);
#else
    foeGfxVkEnumerateSessionFeatures(session, &basicFeatures, nullptr);
#endif

    CHECK(memcmp(&basicFeatures, &basicFeaturesCmp, sizeof(VkPhysicalDeviceFeatures)) == 0);

#ifdef VK_KHR_get_physical_device_properties2
    CHECK(memcmp(&features_1_0, &features_1_0_cmp, sizeof(VkPhysicalDeviceFeatures2)) == 0);
#endif
#ifdef VK_VERSION_1_1
    CHECK(memcmp(&features_1_1, &features_1_1_cmp, sizeof(VkPhysicalDeviceVulkan11Features)) == 0);
#endif
#ifdef VK_VERSION_1_2
    CHECK(memcmp(&features_1_2, &features_1_2_cmp, sizeof(VkPhysicalDeviceVulkan12Features)) == 0);
#endif
#ifdef VK_VERSION_1_3
    CHECK(memcmp(&features_1_3, &features_1_3_cmp, sizeof(VkPhysicalDeviceVulkan13Features)) == 0);
#endif

    foeGfxDestroySession(session);
    foeGfxDestroyRuntime(runtime);
}

TEST_CASE("foeGfxSession(Vulkan) - Passing in an unknown/garbage features struct fails") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;
    uint32_t vkApiVersion;
    uint32_t physicalDeviceCount = 1;
    VkPhysicalDevice vkPhysicalDevice = FOE_NULL_HANDLE;

    vkEnumerateInstanceVersion(&vkApiVersion);

    result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, false, false,
                                   &runtime);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);

    VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                &physicalDeviceCount, &vkPhysicalDevice);
    REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
    REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

    VkBaseInStructure unknownFeatureStruct = {};

    result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, nullptr,
                                   &unknownFeatureStruct, &session);
    CHECK(result.value == FOE_GRAPHICS_VK_ERROR_SESSION_UNKNOWN_FEATURE_STRUCT);
    CHECK(session == FOE_NULL_HANDLE);

    foeGfxDestroyRuntime(runtime);
}

TEST_CASE("foeGfxSession(Vulkan) - Attempting to enable all physical device features") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;
    uint32_t vkApiVersion;
    uint32_t physicalDeviceCount = 1;
    VkPhysicalDevice vkPhysicalDevice = FOE_NULL_HANDLE;

    vkEnumerateInstanceVersion(&vkApiVersion);

    result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, false, false,
                                   &runtime);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);

    VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                &physicalDeviceCount, &vkPhysicalDevice);
    REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
    REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

    VkPhysicalDeviceFeatures basicFeatures;
    memset(&basicFeatures, 1, sizeof(VkPhysicalDeviceFeatures));

#ifdef VK_KHR_get_physical_device_properties2
    VkPhysicalDeviceFeatures2 features_1_0 = {.sType =
                                                  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
    memset(&(features_1_0.features), 1, sizeof(features_1_0.features));
#endif
#ifdef VK_VERSION_1_1
    VkPhysicalDeviceVulkan11Features features_1_1;
    memset(&features_1_1, 1, sizeof(VkPhysicalDeviceVulkan11Features));
    features_1_1.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features_1_1.pNext = nullptr;

    features_1_0.pNext = &features_1_1;
#endif
#ifdef VK_VERSION_1_2
    VkPhysicalDeviceVulkan12Features features_1_2;
    memset(&features_1_2, 1, sizeof(VkPhysicalDeviceVulkan12Features));
    features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features_1_2.pNext = nullptr;

    features_1_1.pNext = &features_1_2;
#endif
#ifdef VK_VERSION_1_3
    VkPhysicalDeviceVulkan13Features features_1_3;
    memset(&features_1_3, 1, sizeof(VkPhysicalDeviceVulkan13Features));
    features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features_1_3.pNext = nullptr;

    features_1_2.pNext = &features_1_3;
#endif

#ifdef VK_KHR_get_physical_device_properties2
    foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, &basicFeatures,
                          &features_1_0, &session);
#else
    foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, &basicFeatures,
                          nullptr, &session);
#endif

    if (session != FOE_NULL_HANDLE)
        foeGfxDestroySession(session);
    foeGfxDestroyRuntime(runtime);
}

TEST_CASE("foeGfxSession(Vulkan) - Checking session MSAA support") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;
    uint32_t vkApiVersion;
    uint32_t physicalDeviceCount = 1;
    VkPhysicalDevice vkPhysicalDevice = FOE_NULL_HANDLE;

    vkEnumerateInstanceVersion(&vkApiVersion);

    result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, false, false,
                                   &runtime);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(runtime != FOE_NULL_HANDLE);

    VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                &physicalDeviceCount, &vkPhysicalDevice);
    REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
    REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

    result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, nullptr,
                                   nullptr, &session);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(session != FOE_NULL_HANDLE);

    VkSampleCountFlags supportedMSAA = foeGfxVkGetSupportedMSAA(session);
    REQUIRE(supportedMSAA != 0);

    VkSampleCountFlags maxMSAA = foeGfxVkGetMaxSupportedMSAA(session);
    REQUIRE((maxMSAA & supportedMSAA) != 0);

    foeGfxDestroySession(session);
    foeGfxDestroyRuntime(runtime);
}

#ifdef VK_VERSION_1_1
TEST_CASE(
    "foeGfxSession(Vulkan) - Attempting to use features for a Vulkan API version greater fails") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    foeGfxSession session = FOE_NULL_HANDLE;
    uint32_t physicalDeviceCount = 1;
    VkPhysicalDevice vkPhysicalDevice = FOE_NULL_HANDLE;

#ifdef VK_VERSION_1_1
    VkPhysicalDeviceVulkan11Features features_1_1 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
#endif
#ifdef VK_VERSION_1_2
    VkPhysicalDeviceVulkan12Features features_1_2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};

    features_1_1.pNext = &features_1_2;
#endif
#ifdef VK_VERSION_1_3
    VkPhysicalDeviceVulkan13Features features_1_3 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};

    features_1_2.pNext = &features_1_3;
#endif

#ifdef VK_VERSION_1_1
    SECTION("Attempting Vulkan V 1.0 with V1.1 features") {
        result = foeGfxVkCreateRuntime(nullptr, 0, VK_API_VERSION_1_0, 0, nullptr, 0, nullptr,
                                       false, false, &runtime);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(runtime != FOE_NULL_HANDLE);

        VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                    &physicalDeviceCount, &vkPhysicalDevice);
        REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
        REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

        result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, nullptr,
                                       &features_1_1, &session);
        REQUIRE(result.value == FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT);
        REQUIRE(session == FOE_NULL_HANDLE);
    }
#endif
#ifdef VK_VERSION_1_2
    SECTION("Attempting Vulkan V 1.1 with V1.2 features") {
        result = foeGfxVkCreateRuntime(nullptr, 0, VK_API_VERSION_1_1, 0, nullptr, 0, nullptr,
                                       false, false, &runtime);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(runtime != FOE_NULL_HANDLE);

        VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                    &physicalDeviceCount, &vkPhysicalDevice);
        REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
        REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

        result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, nullptr,
                                       &features_1_1, &session);
        REQUIRE(result.value == FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT);
        REQUIRE(session == FOE_NULL_HANDLE);
    }
#endif
#ifdef VK_VERSION_1_3
    SECTION("Attempting Vulkan V 1.2 with V1.3 features") {
        result = foeGfxVkCreateRuntime(nullptr, 0, VK_API_VERSION_1_2, 0, nullptr, 0, nullptr,
                                       false, false, &runtime);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(runtime != FOE_NULL_HANDLE);

        VkResult vkRes = vkEnumeratePhysicalDevices(foeGfxVkGetRuntimeInstance(runtime),
                                                    &physicalDeviceCount, &vkPhysicalDevice);
        REQUIRE((vkRes == VK_SUCCESS || vkRes == VK_INCOMPLETE));
        REQUIRE(vkPhysicalDevice != VK_NULL_HANDLE);

        result = foeGfxVkCreateSession(runtime, vkPhysicalDevice, 0, nullptr, 0, nullptr, nullptr,
                                       &features_1_1, &session);
        REQUIRE(result.value == FOE_GRAPHICS_VK_ERROR_SESSION_RUNTIME_NOT_SUPPORT_FEATURE_STRUCT);
        REQUIRE(session == FOE_NULL_HANDLE);
    }
#endif

    foeGfxDestroyRuntime(runtime);
}
#endif
