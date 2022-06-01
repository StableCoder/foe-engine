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

#include <foe/ecs/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeEcsResultToString(foeEcsResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_ECS_SUCCESS)
        RESULT_CASE(FOE_ECS_INCOMPLETE)
        RESULT_CASE(FOE_ECS_NO_MATCH)
        RESULT_CASE(FOE_ECS_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_ECS_ERROR_INDEX_BELOW_MINIMUM)
        RESULT_CASE(FOE_ECS_ERROR_NO_MATCHING_GROUP)
        RESULT_CASE(FOE_ECS_ERROR_NOT_GROUP_ID)
        RESULT_CASE(FOE_ECS_ERROR_OUT_OF_INDEXES)
        RESULT_CASE(FOE_ECS_ERROR_INVALID_ID)
        RESULT_CASE(FOE_ECS_ERROR_INCORRECT_GROUP_ID)
        RESULT_CASE(FOE_ECS_ERROR_INDEX_ABOVE_GENERATED)
        RESULT_CASE(FOE_ECS_ERROR_EMPTY_NAME)
        RESULT_CASE(FOE_ECS_ERROR_ID_ALREADY_EXISTS)
        RESULT_CASE(FOE_ECS_ERROR_NAME_ALREADY_EXISTS)
        RESULT_CASE(FOE_ECS_ERROR_NO_MATCH)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_ECS_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_ECS_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}