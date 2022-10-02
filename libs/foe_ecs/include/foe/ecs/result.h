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
    FOE_ECS_INCOMPLETE = 1000001001,
    FOE_ECS_NO_MATCH = 1000001002,
    FOE_ECS_ERROR_OUT_OF_MEMORY = -1000001001,
    FOE_ECS_ERROR_INDEX_BELOW_MINIMUM = -1000001002,
    FOE_ECS_ERROR_NO_MATCHING_GROUP = -1000001003,
    FOE_ECS_ERROR_NOT_GROUP_ID = -1000001004,
    FOE_ECS_ERROR_OUT_OF_INDEXES = -1000001005,
    FOE_ECS_ERROR_INVALID_ID = -1000001006,
    FOE_ECS_ERROR_INCORRECT_GROUP_ID = -1000001007,
    FOE_ECS_ERROR_INDEX_ABOVE_GENERATED = -1000001008,
    FOE_ECS_ERROR_EMPTY_NAME = -1000001009,
    FOE_ECS_ERROR_ID_ALREADY_EXISTS = -1000001010,
    FOE_ECS_ERROR_NAME_ALREADY_EXISTS = -1000001011,
    FOE_ECS_ERROR_NO_MATCH = -1000001012,
} foeEcsResult;

FOE_ECS_EXPORT void foeEcsResultToString(foeEcsResult value,
                                         char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_ECS_RESULT_H