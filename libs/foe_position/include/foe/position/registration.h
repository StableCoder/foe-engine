// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_POSITION_REGISTRATION_H
#define FOE_POSITION_REGISTRATION_H

#include <foe/error_code.h>
#include <foe/position/export.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_POSITION_EXPORT int foePositionFunctionalityID();

FOE_POSITION_EXPORT foeResultSet foePositionRegisterFunctionality();

FOE_POSITION_EXPORT void foePositionDeregisterFunctionality();

#ifdef __cplusplus
}
#endif

#endif // FOE_POSITION_REGISTRATION_H