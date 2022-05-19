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

#include <foe/imex/error_code.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeImexResultToString(foeImexResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_IMEX_SUCCESS)
        RESULT_CASE(FOE_IMEX_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        // Exporter
        RESULT_CASE(FOE_IMEX_ERROR_EXPORTER_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_ERROR_EXPORTER_NOT_REGISTERED)
        // Importer
        RESULT_CASE(FOE_IMEX_ERROR_IMPORTER_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_ERROR_IMPORTER_NOT_REGISTERED)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_IMEX_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_IMEX_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}