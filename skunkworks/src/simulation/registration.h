// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef BRINGUP_REGISTRATION_H
#define BRINGUP_REGISTRATION_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet foeSkunkworksRegisterFunctionality();

void foeSkunkworksDeregisterFunctionality();

#ifdef __cplusplus
}
#endif

#endif // BRINGUP_REGISTRATION_H