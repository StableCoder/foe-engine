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

#ifndef FOE_ECS_ERROR_CODE_H
#define FOE_ECS_ERROR_CODE_H

#include <foe/ecs/export.h>
#include <foe/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeEcsResult {
    FOE_ECS_SUCCESS = 0,
    FOE_ECS_INCOMPLETE,
    FOE_ECS_NO_MATCH,
    FOE_ECS_ERROR_OUT_OF_MEMORY,
    FOE_ECS_ERROR_INDEX_BELOW_MINIMUM,
    FOE_ECS_ERROR_NO_MATCHING_GROUP,
    FOE_ECS_ERROR_NOT_GROUP_ID,
    FOE_ECS_ERROR_OUT_OF_INDEXES,
    FOE_ECS_ERROR_INVALID_ID,
    FOE_ECS_ERROR_INCORRECT_GROUP_ID,
    FOE_ECS_ERROR_INDEX_ABOVE_GENERATED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_ECS_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeEcsResult;

FOE_ECS_EXPORT void foeEcsResultToString(foeEcsResult value,
                                         char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_ERROR_CODE_H