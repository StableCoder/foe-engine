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
    FOE_ECS_INCOMPLETE = 1,
    FOE_ECS_NO_MATCH = 2,
    FOE_ECS_ERROR_OUT_OF_MEMORY = -1,
    FOE_ECS_ERROR_INDEX_BELOW_MINIMUM = -2,
    FOE_ECS_ERROR_NO_MATCHING_GROUP = -3,
    FOE_ECS_ERROR_NOT_GROUP_ID = -4,
    FOE_ECS_ERROR_OUT_OF_INDEXES = -5,
    FOE_ECS_ERROR_INVALID_ID = -6,
    FOE_ECS_ERROR_INCORRECT_GROUP_ID = -7,
    FOE_ECS_ERROR_INDEX_ABOVE_GENERATED = -8,
    FOE_ECS_ERROR_EMPTY_NAME = -9,
    FOE_ECS_ERROR_ID_ALREADY_EXISTS = -10,
    FOE_ECS_ERROR_NAME_ALREADY_EXISTS = -11,
    FOE_ECS_ERROR_NO_MATCH = -12,
} foeEcsResult;

FOE_ECS_EXPORT void foeEcsResultToString(foeEcsResult value,
                                         char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_RESULT_H