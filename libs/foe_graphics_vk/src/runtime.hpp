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

#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <foe/graphics/runtime.hpp>
#include <vulkan/vulkan.h>

struct foeGfxVkRuntime {
    VkInstance instance{VK_NULL_HANDLE};
    VkDebugReportCallbackEXT debugCallback{VK_NULL_HANDLE};
};

FOE_DEFINE_HANDLE_CASTS(runtime, foeGfxVkRuntime, foeGfxRuntime)

#endif // RUNTIME_HPP