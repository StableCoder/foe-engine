// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef XR_RESULT_H
#define XR_RESULT_H

#include <foe/error_code.h>
#include <openxr/openxr.h>

#ifdef __cplusplus
extern "C" {
#endif

void XrResultToString(XrResult value, char buffer[FOE_MAX_RESULT_STRING_SIZE]);

inline foeResult xr_to_foeResult(XrResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)XrResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // XR_RESULT_H