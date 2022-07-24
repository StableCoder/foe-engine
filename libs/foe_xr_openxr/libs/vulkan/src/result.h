// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/xr/openxr/vk/result.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResultSet to_foeResult(foeOpenXrVkResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeOpenXrVkResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H