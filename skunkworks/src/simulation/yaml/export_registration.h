// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SIMULATION_YAML_EXPORT_REGISTRATION_H
#define SIMULATION_YAML_EXPORT_REGISTRATION_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet foeBringupYamlRegisterExporters();

void foeBringupYamlDeregisterExporters();

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_YAML_EXPORT_REGISTRATION_H