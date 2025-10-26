// Copyright (C) 2021-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef REGISTER_BASIC_FUNCTIONALITY_H
#define REGISTER_BASIC_FUNCTIONALITY_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet registerBasicFunctionality();

void deregisterBasicFunctionality();

#ifdef __cplusplus
}
#endif

#endif // REGISTER_BASIC_FUNCTIONALITY_H