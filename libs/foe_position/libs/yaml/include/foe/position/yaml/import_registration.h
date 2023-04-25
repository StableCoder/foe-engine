// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_YAML_IMPORT_REGISTRATION_H
#define FOE_POSITION_YAML_IMPORT_REGISTRATION_H

#include <foe/position/yaml/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_POSITION_YAML_EXPORT
foeResultSet foePositionYamlRegisterImporters();

FOE_POSITION_YAML_EXPORT
void foePositionYamlDeregisterImporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_YAML_IMPORT_REGISTRATION_H