// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_YAML_RESULT_H
#define FOE_POSITION_YAML_RESULT_H

#include <foe/position/yaml/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foePositionYamlResult {
    FOE_POSITION_YAML_SUCCESS = 0,
    FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_IMPORTER = -1000016001,
    FOE_POSITION_YAML_ERROR_FAILED_TO_REGISTER_3D_EXPORTER = -1000016002,
} foePositionYamlResult;

FOE_POSITION_YAML_EXPORT void foePositionYamlResultToString(
    foePositionYamlResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_YAML_RESULT_H