// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeStateImportResult {
    FOE_STATE_IMPORT_SUCCESS = 0,
    FOE_STATE_IMPORT_ERROR_NO_IMPORTER,
    FOE_STATE_IMPORT_ERROR_IMPORTING_DEPENDENCIES,
    FOE_STATE_IMPORT_ERROR_DUPLICATE_DEPENDENCIES,
    FOE_STATE_IMPORT_ERROR_TRANSITIVE_DEPENDENCIES_UNFULFILLED,
    FOE_STATE_IMPORT_ERROR_ECS_GROUP_SETUP_FAILURE,
    FOE_STATE_IMPORT_ERROR_IMPORTING_INDEX_DATA,
    FOE_STATE_IMPORT_ERROR_IMPORTING_RESOURCE,
    FOE_STATE_IMPORT_ERROR_NO_COMPONENT_IMPORTER,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_STATE_IMPORT_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeStateImportResult;

void foeStateImportResultToString(foeStateImportResult value,
                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline foeResultSet to_foeResult(foeStateImportResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeStateImportResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H