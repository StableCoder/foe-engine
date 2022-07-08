// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/resource/error_code.h>

inline foeResult to_foeResult(foeResourceResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeResourceResultToString,
    };

    return result;
}

#endif // RESULT_H