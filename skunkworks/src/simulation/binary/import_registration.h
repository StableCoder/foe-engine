// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef SIMULATION_BINARY_IMPORT_REGISTRATION_H
#define SIMULATION_BINARY_IMPORT_REGISTRATION_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet foeBringupBinaryRegisterImporters();

void foeBringupBinaryDeregisterImporters();

#ifdef __cplusplus
}
#endif

#endif // SIMULATION_BINARY_IMPORT_REGISTRATION_H