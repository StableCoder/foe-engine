// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef RESULT_H
#define RESULT_H

#include <foe/imex/error_code.h>
#include <foe/imex/yaml/error_code.h>

#ifdef __cplusplus
extern "C" {
#endif

inline foeResultSet to_foeResult(foeImexYamlResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeImexYamlResultToString,
    };

    return result;
}

inline foeResultSet imex_to_foeResult(foeImexResult value) {
    foeResultSet result = {
        .value = value,
        .toString = (PFN_foeResultToString)foeImexResultToString,
    };

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // RESULT_H