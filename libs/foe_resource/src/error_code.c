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

#include <foe/resource/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeResourceResultToString(foeResourceResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_RESOURCE_SUCCESS)
        RESULT_CASE(FOE_RESOURCE_ERROR_NOT_FOUND)
        // General
        RESULT_CASE(FOE_RESOURCE_ERROR_OUT_OF_HOST_MEMORY)
        // Records
        RESULT_CASE(FOE_RESOURCE_CANNOT_UNDO)
        RESULT_CASE(FOE_RESOURCE_CANNOT_REDO)
        RESULT_CASE(FOE_RESOURCE_NO_MODIFIED_RECORD)
        RESULT_CASE(FOE_RESOURCE_ERROR_EXISTING_RECORD)
        RESULT_CASE(FOE_RESOURCE_ERROR_NON_EXISTING_RECORD)
        RESULT_CASE(FOE_RESOURCE_ERROR_CANNOT_REMOVE_NON_PERSISTENT_RECORDS)
        RESULT_CASE(FOE_RESOURCE_ERROR_NO_RECORDS)
        // Resource Specific
        RESULT_CASE(FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED)
        // CreateInfo Specific
        RESULT_CASE(FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_RESOURCE_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_RESOURCE_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}