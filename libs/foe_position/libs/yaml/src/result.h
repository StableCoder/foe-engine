// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/position/yaml/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResult to_foeResult(foePositionYamlResult value) {
    foeResult result = {
        .value = value,
        .toString = (PFN_foeResultToString)foePositionYamlResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H