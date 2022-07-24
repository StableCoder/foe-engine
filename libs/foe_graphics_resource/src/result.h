// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/graphics/resource/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResultSet to_foeResult(foeGraphicsResourceResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeGraphicsResourceResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H