// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef NETWORK_INITIALIZATION_H
#define NETWORK_INITIALIZATION_H

#include <foe/result.h>

#ifdef __cplusplus
extern "C" {
#endif

foeResultSet initializeNetworkStack();

void deinitializeNetworkStack();

#ifdef __cplusplus
}
#endif

#endif // NETWORK_INITIALIZATION_H