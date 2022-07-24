// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SIMULATION_YAML_IMPORT_REGISTRATION_HPP
#define SIMULATION_YAML_IMPORT_REGISTRATION_HPP

#include <foe/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet foeBringupYamlRegisterImporters();

void foeBringupYamlDeregisterImporters();

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_YAML_IMPORT_REGISTRATION_HPP