/*
    Copyright (C) 2022 George Cave.

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

#ifndef VK_RESULT_H
#define VK_RESULT_H

#include <foe/error_code.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void VkResultToString(VkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline foeResult vk_to_foeResult(VkResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)NULL,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // VK_RESULT_H