/*
    Copyright (C) 2021 George Cave - gcave@stablecoder.ca

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

#include <catch.hpp>
#include <foe/wsi/vulkan.h>

TEST_CASE("WSI-GLFW - Vulkan Extensions") {
    uint32_t count{0};
    char const **ppExtensionNames{nullptr};
    CHECK_FALSE(std::error_code{foeWsiWindowGetVulkanExtensions(&count, &ppExtensionNames)});

    CHECK(count > 0);
    CHECK(ppExtensionNames != nullptr);
}