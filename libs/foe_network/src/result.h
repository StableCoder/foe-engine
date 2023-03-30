// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/network/result.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline foeResultSet to_foeResult(foeNetworkResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeNetworkResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H