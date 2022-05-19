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

#ifndef ERROR_CODE_H
#define ERROR_CODE_H

#include <foe/error_code.h>

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

#ifdef __cplusplus
}
#endif

#endif // ERROR_CODE_H