// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_ECS_RESULT_H
#define FOE_ECS_RESULT_H

#include <foe/ecs/export.h>
#include <foe/result.h>

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
    FOE_ECS_ERROR_EMPTY_NAME,
    FOE_ECS_ERROR_ID_ALREADY_EXISTS,
    FOE_ECS_ERROR_NAME_ALREADY_EXISTS,
    FOE_ECS_ERROR_NO_MATCH,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_ECS_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeEcsResult;

FOE_ECS_EXPORT void foeEcsResultToString(foeEcsResult value,
                                         char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_RESULT_H