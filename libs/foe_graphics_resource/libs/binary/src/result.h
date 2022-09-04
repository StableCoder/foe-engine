// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/graphics/resource/binary/result.h>

#ifdef __cplusplus
extern "C" {
#endif

inline static foeResultSet to_foeResult(foeGraphicsResourceBinaryResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeGraphicsResourceBinaryResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H