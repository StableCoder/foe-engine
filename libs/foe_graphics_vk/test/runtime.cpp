// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
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
        CHECK(strLen == 0);

        strLen = UINT32_MAX;
        result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, nullptr);
        CHECK(result.value == FOE_SUCCESS);
        CHECK(strLen == 0);
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
                CHECK(std::string_view{strBuffer} == "VK_LAYER_KHRONOS_validation");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 28);
                CHECK(std::string_view{strBuffer} == "VK_LAYER_KHRONOS_validation");
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
        CHECK(strLen == 20);

        SECTION("Enumerating Extensions") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 20);
                CHECK(std::string_view{strBuffer} == "VK_EXT_debug_report");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 20);
                CHECK(std::string_view{strBuffer} == "VK_EXT_debug_report");
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
                CHECK(std::string_view{strBuffer} == "VK_LAYER_KHRONOS_validation");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeLayers(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 28);
                CHECK(std::string_view{strBuffer} == "VK_LAYER_KHRONOS_validation");
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
        CHECK(strLen == 20);

        SECTION("Enumerating Extensions") {
            SECTION("Oversized buffer provided") {
                strLen = sizeof(strBuffer);
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 20);
                CHECK(std::string_view{strBuffer} == "VK_EXT_debug_report");
            }
            SECTION("Exact buffer provided") {
                result = foeGfxVkEnumerateRuntimeExtensions(runtime, &strLen, strBuffer);
                CHECK(result.value == FOE_SUCCESS);
                CHECK(strLen == 20);
                CHECK(std::string_view{strBuffer} == "VK_EXT_debug_report");
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