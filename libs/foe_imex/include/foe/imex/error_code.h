/*
    Copyright (C) 2021-2022 George Cave.

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

#ifndef FOE_IMEX_ERROR_CODE_H
#define FOE_IMEX_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/imex/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeImexResult {
    FOE_IMEX_SUCCESS = 0,
    FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED,
    // Exporter
    FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED,
    FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED,
    // Importer
    FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED,
    FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_IMEX_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeImexResult;

FOE_IMEX_EXPORT void foeImexResultToString(foeImexResult value,
                                           char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_ERROR_CODE_H