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

#ifndef FOE_GRAPHICS_VK_RUNTIME_HPP
#define FOE_GRAPHICS_VK_RUNTIME_HPP

#include <foe/graphics/export.h>
#include <foe/graphics/runtime.hpp>

#include <string>
#include <system_error>
#include <vector>

FOE_GFX_EXPORT std::error_code foeGfxVkCreateRuntime(char const *applicationName,
                                                     uint32_t applicationVersion,
                                                     std::vector<std::string> layers,
                                                     std::vector<std::string> extensions,
                                                     bool validation,
                                                     bool debugLogging,
                                                     foeGfxRuntime *pRuntime);

#include <vulkan/vulkan.h>

FOE_GFX_EXPORT VkInstance foeGfxVkGetInstance(foeGfxRuntime runtime);

#endif // FOE_GRAPHICS_VK_RUNTIME_HPP