// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include "error_code.h"

#ifdef __cplusplus
extern "C" {
#endif

inline foeResult to_foeResult(foeStateImportResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeStateImportResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H