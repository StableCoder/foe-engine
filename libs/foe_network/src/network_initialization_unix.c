// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "network_initialization.h"

#include "result.h"

foeResultSet initializeNetworkStack() { return to_foeResult(FOE_NETWORK_SUCCESS); }

void deinitializeNetworkStack() {}
