// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/delimited_string.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

bool foeCreateDelimitedString(uint32_t srcCount,
                              char const *const *ppSrc,
                              uint32_t *pDstLength,
                              char *pDst) {
    size_t totalLength = 0;
    for (size_t i = 0; i < srcCount; ++i) {
        // Length of the string plus space for the null character
        size_t const len = strlen(ppSrc[i]);
        if (len > 0)
            totalLength += len + 1;
    }

    if (pDst == NULL) {
        // If no destination was provided, then return the required memory size
        *pDstLength = totalLength;
        return true;
    }

    uint32_t usedLength = 0;
    if (totalLength != 0) {
        for (size_t i = 0; i < srcCount; ++i) {
            size_t len = strlen(ppSrc[i]);

            // Skip if a zero-length string
            if (len == 0)
                continue;
            // If there's not enough space for the next string, break out early
            if (usedLength + len + 1 > *pDstLength)
                break;

            // There's enough space, copy the string
            memcpy(pDst, ppSrc[i], len);
            pDst += len;
            *pDst = '\0';
            ++pDst;
            usedLength += len + 1;
        }
    }

    // Set the amount written to the destination
    *pDstLength = usedLength;

    return (usedLength == totalLength);
}

bool foeCopyDelimitedString(uint32_t srcLength,
                            char const *pSrc,
                            uint32_t *pDstLength,
                            char *pDst) {
    if (pDst == NULL) {
        // If the destination pointer wasn't provided, then update and return the length required
        *pDstLength = srcLength;
        return true;
    }

    if (*pDstLength >= srcLength) {
        memcpy(pDst, pSrc, srcLength);
        *pDstLength = srcLength;

        return true;
    } else {
        uint32_t offset = *pDstLength - 1;
        while (offset != 0 && pSrc[offset] != '\0') {
            --offset;
        }

        if (offset != 0) {
            // Increment by one to capture the ending null character
            ++offset;
            memcpy(pDst, pSrc, offset);
        }
        *pDstLength = offset;

        return false;
    }
}

bool foeIndexedDelimitedString(uint32_t srcLength,
                               char const *pSrc,
                               uint32_t index,
                               char const **ppStr) {
    // If there's no input, there's nothing to cut
    if (srcLength == 0)
        return false;

    char const *const pEnd = pSrc + srcLength;
    uint32_t curIndex = 0;

    // Go through the string, finding the delimit points of the strings
    while (pSrc != pEnd && curIndex != index) {
        if (*pSrc == '\0') {
            curIndex++;
        }
        ++pSrc;
    }

    // Aren't enough strings
    if (curIndex != index)
        return false;

    // Output the indexed cut string start
    *ppStr = pSrc;

    return true;
}