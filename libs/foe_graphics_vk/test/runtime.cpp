// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/graphics/vk/result.h>
#include <foe/graphics/vk/runtime.h>

TEST_CASE("foeGfxRuntime(Vulkan)") {
    foeResultSet result;
    foeGfxRuntime runtime = FOE_NULL_HANDLE;
    uint32_t vkApiVersion;

    uint32_t strLen;
    char strBuffer[512];

    vkEnumerateInstanceVersion(&vkApiVersion);

    SECTION("No extensions or layers, no validation, no debug logging") {
        result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, false,
                                       false, &runtime);
        REQUIRE(result.value == FOE_SUCCESS);
        CHECK(runtime != FOE_NULL_HANDLE);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
        // macOS has some base required extensions after Vulkan 216
        CHECK(strLen == strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                            strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2);
#else
        CHECK(strLen == 0);
#endif
    }
    SECTION("Validation Layer and Debugging Extension enabled") {
        result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 0, nullptr, 0, nullptr, true, true,
                                       &runtime);
        REQUIRE(result.value == FOE_SUCCESS);
        CHECK(runtime != FOE_NULL_HANDLE);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 28);

        SECTION("Enumerating Layers") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 28);
                CHECK(std::string{strBuffer} == "VK_LAYER_KHRONOS_validation");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 28);
                CHECK(std::string{strBuffer} == "VK_LAYER_KHRONOS_validation");
            }
            SECTION("Undersized buffer provided") {
                strLen = 10;
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_GRAPHICS_VK_INCOMPLETE);
                CHECK(strLen == 0);
            }
        }

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
        CHECK(strLen == 20 + (strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                              strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2));
#else
        CHECK(strLen == 20);
#endif

        SECTION("Enumerating Extensions") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
                CHECK(strLen ==
                      20 + (strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                            strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2));
#else
                CHECK(strLen == 20);
#endif
                CHECK(std::string{strBuffer} == "VK_EXT_debug_report");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
                CHECK(strLen ==
                      20 + (strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                            strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2));
#else
                CHECK(strLen == 20);
#endif
                CHECK(std::string{strBuffer} == "VK_EXT_debug_report");
            }
            SECTION("Undersized buffer provided") {
                strLen = 10;
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_GRAPHICS_VK_INCOMPLETE);
                CHECK(strLen == 0);
            }
        }
    }

    SECTION("Providing layers and extensions") {
        char layer[] = "VK_LAYER_KHRONOS_validation";
        char *ppLayers = layer;
        char extension[] = "VK_EXT_debug_report";
        char *ppExtensions = extension;

        result = foeGfxVkCreateRuntime(nullptr, 0, vkApiVersion, 1, &ppLayers, 1, &ppExtensions,
                                       false, false, &runtime);
        REQUIRE(result.value == FOE_SUCCESS);
        CHECK(runtime != FOE_NULL_HANDLE);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 28);

        SECTION("Enumerating Layers") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 28);
                CHECK(std::string{strBuffer} == "VK_LAYER_KHRONOS_validation");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 28);
                CHECK(std::string{strBuffer} == "VK_LAYER_KHRONOS_validation");
            }
            SECTION("Undersized buffer provided") {
                strLen = 10;
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_GRAPHICS_VK_INCOMPLETE);
                CHECK(strLen == 0);
            }
        }

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
        CHECK(strLen == 20 + (strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                              strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2));
#else
        CHECK(strLen == 20);
#endif

        SECTION("Enumerating Extensions") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
                CHECK(strLen ==
                      20 + (strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                            strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2));
#else
                CHECK(strLen == 20);
#endif
                CHECK(std::string{strBuffer} == "VK_EXT_debug_report");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
#if defined(VK_USE_PLATFORM_MACOS_MVK) && (VK_HEADER_VERSION >= 216)
                CHECK(strLen ==
                      20 + (strlen(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) +
                            strlen(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) + 2));
#else
                CHECK(strLen == 20);
#endif
                CHECK(std::string{strBuffer} == "VK_EXT_debug_report");
            }
            SECTION("Undersized buffer provided") {
                strLen = 10;
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_GRAPHICS_VK_INCOMPLETE);
                CHECK(strLen == 0);
            }
        }
    }

    CHECK(foeGfxVkGetRuntimeInstance(runtime) != VK_NULL_HANDLE);
    CHECK(foeGfxVkEnumerateApiVersion(runtime) == vkApiVersion);

    foeGfxDestroyRuntime(runtime);
}