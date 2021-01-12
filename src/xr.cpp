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

#include "xr.hpp"

#include <foe/graphics/backend.h>
#include <foe/xr/vulkan.hpp>

std::error_code createXrRuntime(bool debugLogging, foeXrRuntime *pRuntime) {
    std::vector<std::string> layers;
    std::vector<std::string> extensions;

    if (foeGfxGetBackend() == FOE_GFX_BACKEND_VULKAN) {
        extensions.emplace_back(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
    } else {
        std::abort();
    }

    return pRuntime->createRuntime("FoE Engine", 0, layers, extensions, debugLogging);
}