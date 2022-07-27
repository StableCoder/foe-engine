// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/imex/yaml/result.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESULT_CASE(X)                                                                             \
    case X:                                                                                        \
        pResultStr = #X;                                                                           \
        break;

void foeImexYamlResultToString(foeImexYamlResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    char const *pResultStr = NULL;

    switch (value) {
        RESULT_CASE(FOE_IMEX_YAML_SUCCESS)
        RESULT_CASE(FOE_IMEX_YAML_INCOMPLETE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_OUT_OF_MEMORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED)
        // Importer
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_FIND_RESOURCE_IMPORTER)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_IMPORT_RESOURCE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_FIND_COMPONENT_IMPORTER)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_IMPORT_COMPONENT)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DEPENDENCIES)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_FILE_NOT_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_ENTITY_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_EXIST)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_RESOURCE_FILE_NOT_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_RESOURCE_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_OPEN_FILE)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DATA)
        // Exporter
        RESULT_CASE(FOE_IMEX_YAML_ERROR_EXPORTER_ALREADY_REGISTERED)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_EXPORTER_NOT_REGISTERED)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA)
        RESULT_CASE(FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA)

    default:
        if (value > 0) {
            sprintf(buffer, "FOE_IMEX_YAML_UNKNOWN_SUCCESS_%i", value);
        } else {
            value = abs(value);
            sprintf(buffer, "FOE_IMEX_YAML_UNKNOWN_ERROR_%i", value);
        }
    }

    if (pResultStr != NULL) {
        assert(strlen(pResultStr) <= FOE_MAX_RESULT_STRING_SIZE);

        strcpy(buffer, pResultStr);
    }
}