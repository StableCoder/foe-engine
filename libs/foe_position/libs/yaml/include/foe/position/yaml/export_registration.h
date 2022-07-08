// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_YAML_EXPORT_REGISTRATION_H
#define FOE_POSITION_YAML_EXPORT_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/position/yaml/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_POSITION_YAML_EXPORT foeResult foePositionYamlRegisterExporters();

FOE_POSITION_YAML_EXPORT void foePositionYamlDeregisterExporters();

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_YAML_EXPORT_REGISTRATION_H