// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/ecs/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResult to_foeResult(foeEcsResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeEcsResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H