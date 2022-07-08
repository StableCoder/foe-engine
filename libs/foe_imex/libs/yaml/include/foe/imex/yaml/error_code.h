// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_YAML_ERROR_CODE_H
#define FOE_IMEX_YAML_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/imex/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeImexYamlResult {
    FOE_IMEX_YAML_SUCCESS = 0,
    FOE_IMEX_YAML_INCOMPLETE,
    FOE_IMEX_YAML_ERROR_FUNCTIONALITY_ALREADY_REGISTERED,
    FOE_IMEX_YAML_ERROR_FUNCTIONALITY_NOT_REGISTERED,
    // Importer
    FOE_IMEX_YAML_ERROR_PATH_NOT_DIRECTORY,
    FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_EXIST,
    FOE_IMEX_YAML_ERROR_DEPENDENCIES_FILE_NOT_REGULAR_FILE,
    FOE_IMEX_YAML_ERROR_FAILED_TO_READ_DEPENDENCIES,
    FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_EXIST,
    FOE_IMEX_YAML_ERROR_RESOURCE_INDEX_FILE_NOT_REGULAR_FILE,
    FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_EXIST,
    FOE_IMEX_YAML_ERROR_ENTITY_INDEX_FILE_NOT_REGULAR_FILE,
    FOE_IMEX_YAML_ERROR_RESOURCE_DIRECTORY_NOT_DIRECTORY,
    FOE_IMEX_YAML_ERROR_ENTITY_DIRECTORY_NOT_DIRECTORY,
    FOE_IMEX_YAML_ERROR_EXTERNAL_DIRECTORY_NOT_DIRECTORY,
    // Exporter
    FOE_IMEX_YAML_ERROR_EXPORTER_ALREADY_REGISTERED,
    FOE_IMEX_YAML_ERROR_EXPORTER_NOT_REGISTERED,
    FOE_IMEX_YAML_ERROR_DESTINATION_NOT_DIRECTORY,
    FOE_IMEX_YAML_ERROR_FAILED_TO_PERFORM_FILESYSTEM_OPERATION,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_DEPENDENCIES,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_INDEX_DATA,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_INDEX_DATA,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_RESOURCE_DATA,
    FOE_IMEX_YAML_ERROR_FAILED_TO_WRITE_COMPONENT_DATA,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_IMEX_YAML_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foeImexYamlResult;

FOE_IMEX_YAML_EXPORT void foeImexYamlResultToString(foeImexYamlResult value,
                                                    char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_YAML_ERROR_CODE_H