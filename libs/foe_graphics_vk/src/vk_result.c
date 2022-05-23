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

#include "vk_result.h"

#define VK_RESULT_TO_STRING_CONFIG_MAIN
#include <vk_result_to_string.h>

#include <assert.h>
#include <string.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

_Static_assert(VK_SUCCESS == FOE_SUCCESS, "VK_SUCCESS must equal FOE_SUCCESS");

void VkResultToString(VkResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = VkResult_to_string(value);

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    } else {
        if (value > 0) {
            sprintf(buffer, "VK_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "VK_UNKNOWN_ERROR_%i", value);
        }
    }
}
