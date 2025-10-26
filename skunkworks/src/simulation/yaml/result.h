// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum foeBringupYamlResult {
    FOE_SKUNKWORKS_YAML_SUCCESS = 0,
    FOE_SKUNKWORKS_YAML_ERROR_UNSPECIFIED = -1000024001,
    FOE_SKUNKWORKS_YAML_ERROR_ARMATURE_POOL_NOT_FOUND = -1000024002,
    FOE_SKUNKWORKS_YAML_ERROR_ARMATURE_RESOURCE_ALREADY_EXISTS = -1000024003,
    FOE_SKUNKWORKS_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_IMPORTER = -1000024004,
    FOE_SKUNKWORKS_YAML_ERROR_FAILED_TO_REGISTER_ARMATURE_STATE_IMPORTER = -1000024005,
    FOE_SKUNKWORKS_YAML_ERROR_FAILED_TO_REGISTER_RENDER_STATE_IMPORTER = -1000024006,
    FOE_SKUNKWORKS_YAML_ERROR_FAILED_TO_REGISTER_CAMERA_IMPORTER = -1000024007,
    FOE_SKUNKWORKS_YAML_ERROR_FAILED_TO_REGISTER_RESOURCE_EXPORTERS = -1000024008,
    FOE_SKUNKWORKS_YAML_ERROR_FAILED_TO_REGISTER_COMPONENT_EXPORTERS = -1000024009,
} foeBringupYamlResult;

void foeBringupYamlResultToString(foeBringupYamlResult value,
                                  char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline foeResultSet to_foeResult(foeBringupYamlResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeBringupYamlResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H