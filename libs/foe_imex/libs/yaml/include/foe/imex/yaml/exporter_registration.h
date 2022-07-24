// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_IMEX_YAML_EXPORTER_REGISTRATION_H
#define FOE_IMEX_YAML_EXPORTER_REGISTRATION_H

#include <foe/imex/yaml/export.h>
#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_IMEX_YAML_EXPORT foeResultSet foeImexYamlRegisterExporter();
FOE_IMEX_YAML_EXPORT void foeImexYamlDeregisterExporter();

#ifdef __cplusplus
}
#endif

#endif // FOE_IMEX_YAML_EXPORTER_REGISTRATION_H