// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_MEMORY_ALIGNMENT_H
#define FOE_MEMORY_ALIGNMENT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

inline size_t foeGetAlignedSize(size_t alignment, size_t size) {
    size_t total = size;
    size_t const alignmentDiff = size % alignment;
    if (alignmentDiff != 0) {
        total += alignment - alignmentDiff;
    }

    return total;
}

#ifdef __cplusplus
}
#endif

#endif // FOE_MEMORY_ALIGNMENT_H