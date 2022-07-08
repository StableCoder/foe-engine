// Copyright (C) 2021-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/wsi/vulkan.h>

TEST_CASE("WSI-GLFW - Vulkan Extensions") {
    uint32_t count{0};
    char const **ppExtensionNames{nullptr};
    CHECK(foeWsiWindowGetVulkanExtensions(&count, &ppExtensionNames).value == FOE_SUCCESS);

    CHECK(count > 0);
    CHECK(ppExtensionNames != nullptr);
}