// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/crypto/result.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline foeResultSet to_foeResult(foeCryptoResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeCryptoResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H