// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_YAML_ERROR_CODE_H
#define FOE_POSITION_YAML_ERROR_CODE_H

#include <foe/error_code.h>
#include <foe/position/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foePositionYamlResult {
    FOE_POSITION_YAML_SUCCESS = 0,
    // Position3D Component
    FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER,
    FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_EXPORTER,

    // Need to have a negative enum value to prevent treatment as a flag
    FOE_POSITION_ERROR_NEGATIVE_VALUE = FOE_RESULT_MIN_ENUM,
} foePositionYamlResult;

FOE_POSITION_YAML_EXPORT void foePositionYamlResultToString(
    foePositionYamlResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_YAML_ERROR_CODE_H