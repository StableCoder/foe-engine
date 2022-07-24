// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/xr/openxr/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResultSet to_foeResult(foeOpenXrResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeOpenXrResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H