// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/utf_string_conversion.h>

#include "utf_character_conversion.h"

foeResult foe_utf8_to_utf16(size_t *pSrcCount,
                            uint8_t const *pSrc,
                            size_t *pDstCount,
                            uint16_t *pDst) {
    size_t srcCount = *pSrcCount;
    size_t dstCount = 0;

    if (srcCount == 0) {
        // nothing to process
        *pDstCount = 0;
        return FOE_SUCCESS;
    } else if (pDst && *pDstCount == 0) {
        // no room in destination
        *pSrcCount = 0;
        return FOE_INCOMPLETE;
    }

    foeMultiByteState state = {0};
    foeResult result;
    size_t uncommittedReadSize = 0;
    size_t uncommittedWriteSize = 0;
    uint16_t out;

    // process until input exhausted
    while (srcCount != 0) {
        result = foe_utf8_to_utf16_ch(*pSrc, &out, &state);

        if (result == FOE_ERROR_UTF_MALFORMED_DATA || result == FOE_ERROR_UTF_MB_INCOMPLETE ||
            result == FOE_ERROR_UTF_INVALID_CODEPOINT) {
            break;

        } else if (result == FOE_AWAITING_INPUT) {
            ++uncommittedReadSize;
            --srcCount;
            ++pSrc;
            continue;
        }

        if (result == FOE_INCOMPLETE) {
            // outputting multi-byte
            ++uncommittedWriteSize;
        } else {
            // FOE_SUCCESS
            --srcCount;
            ++pSrc;
            uncommittedReadSize = 0;
            uncommittedWriteSize = 0;
        }

        ++dstCount;
        if (pDst) {
            *pDst = out;
            ++pDst;
            if (dstCount == *pDstCount)
                // destination buffer full
                break;
        }
    }

    if (result != FOE_SUCCESS) {
        // failed to process everything
        srcCount += uncommittedReadSize;
        dstCount -= uncommittedWriteSize;
        if (result == FOE_AWAITING_INPUT)
            result = FOE_ERROR_UTF_MB_INCOMPLETE;
    }

    // set the final counts
    *pSrcCount -= srcCount;
    *pDstCount = dstCount;

    return result;
}

foeResult foe_utf16_to_utf8(size_t *pSrcCount,
                            uint16_t const *pSrc,
                            size_t *pDstCount,
                            uint8_t *pDst) {
    size_t srcCount = *pSrcCount;
    size_t dstCount = 0;

    if (srcCount == 0) {
        // nothing to process
        *pDstCount = 0;
        return FOE_SUCCESS;
    } else if (pDst && *pDstCount == 0) {
        // no room in destination
        *pSrcCount = 0;
        return FOE_INCOMPLETE;
    }

    foeMultiByteState state = {0};
    foeResult result;
    size_t uncommittedReadSize = 0;
    size_t uncommittedWriteSize = 0;
    uint8_t out;

    // process until input exhausted
    while (srcCount != 0) {
        result = foe_utf16_to_utf8_ch(*pSrc, &out, &state);

        if (result == FOE_ERROR_UTF_MALFORMED_DATA || result == FOE_ERROR_UTF_MB_INCOMPLETE) {
            break;

        } else if (result == FOE_AWAITING_INPUT) {
            ++uncommittedReadSize;
            --srcCount;
            ++pSrc;
            continue;
        }

        if (result == FOE_INCOMPLETE) {
            // outputting multi-byte
            ++uncommittedWriteSize;
        } else {
            // FOE_SUCCESS
            --srcCount;
            ++pSrc;
            uncommittedReadSize = 0;
            uncommittedWriteSize = 0;
        }

        ++dstCount;
        if (pDst) {
            *pDst = out;
            ++pDst;
            if (dstCount == *pDstCount)
                // destination buffer full
                break;
        }
    }

    if (result != FOE_SUCCESS) {
        // failed to process everything
        srcCount += uncommittedReadSize;
        dstCount -= uncommittedWriteSize;
        if (result == FOE_AWAITING_INPUT)
            result = FOE_ERROR_UTF_MB_INCOMPLETE;
    }

    // set the final counts
    *pSrcCount -= srcCount;
    *pDstCount = dstCount;

    return result;
}

foeResult foe_utf16_to_utf32(size_t *pSrcCount,
                             uint16_t const *pSrc,
                             size_t *pDstCount,
                             uint32_t *pDst) {
    size_t srcCount = *pSrcCount;
    size_t dstCount = 0;

    if (*pSrcCount == 0) {
        // nothing to process
        *pDstCount = 0;
        return FOE_SUCCESS;
    } else if (pDst && *pDstCount == 0) {
        // no room in destination
        *pSrcCount = 0;
        return FOE_INCOMPLETE;
    }

    foeMultiByteState state = {0};
    foeResult result;
    size_t uncommittedReadSize = 0;
    uint32_t out;

    // process until input exhausted
    while (srcCount != 0) {
        result = foe_utf16_to_utf32_ch(*pSrc, &out, &state);

        if (result == FOE_ERROR_UTF_MALFORMED_DATA || result == FOE_ERROR_UTF_MB_INCOMPLETE) {
            break;

        } else if (result == FOE_AWAITING_INPUT) {
            ++uncommittedReadSize;
            --srcCount;
            ++pSrc;
            continue;
        }

        // FOE_SUCCESS
        --srcCount;
        ++pSrc;
        uncommittedReadSize = 0;

        ++dstCount;
        if (pDst) {
            *pDst = out;
            ++pDst;
            if (dstCount == *pDstCount)
                // destination buffer full
                break;
        }
    }

    if (result != FOE_SUCCESS) {
        // failed to process everything
        srcCount += uncommittedReadSize;
        if (result == FOE_AWAITING_INPUT)
            result = FOE_ERROR_UTF_MB_INCOMPLETE;
    }

    if (result == FOE_SUCCESS && srcCount != 0)
        result = FOE_INCOMPLETE;

    // set the final counts
    *pSrcCount -= srcCount;
    *pDstCount = dstCount;

    return result;
}

foeResult foe_utf8_to_utf32(size_t *pSrcCount,
                            uint8_t const *pSrc,
                            size_t *pDstCount,
                            uint32_t *pDst) {
    size_t srcCount = *pSrcCount;
    size_t dstCount = 0;

    if (*pSrcCount == 0) {
        // nothing to process
        *pDstCount = 0;
        return FOE_SUCCESS;
    } else if (pDst && *pDstCount == 0) {
        // no room in destination
        *pSrcCount = 0;
        return FOE_INCOMPLETE;
    }

    foeMultiByteState state = {0};
    foeResult result;
    size_t uncommittedReadSize = 0;
    uint32_t out;

    // process until input exhausted
    while (srcCount != 0) {
        result = foe_utf8_to_utf32_ch(*pSrc, &out, &state);

        if (result == FOE_ERROR_UTF_MALFORMED_DATA || result == FOE_ERROR_UTF_MB_INCOMPLETE ||
            result == FOE_ERROR_UTF_INVALID_CODEPOINT) {
            break;

        } else if (result == FOE_AWAITING_INPUT) {
            ++uncommittedReadSize;
            --srcCount;
            ++pSrc;
            continue;
        }

        // FOE_SUCCESS
        --srcCount;
        ++pSrc;
        uncommittedReadSize = 0;

        ++dstCount;
        if (pDst) {
            *pDst = out;
            ++pDst;
            if (dstCount == *pDstCount)
                // destination buffer full
                break;
        }
    }

    if (result != FOE_SUCCESS) {
        // failed to process everything
        srcCount += uncommittedReadSize;
        if (result == FOE_AWAITING_INPUT)
            result = FOE_ERROR_UTF_MB_INCOMPLETE;
    }

    if (result == FOE_SUCCESS && srcCount != 0)
        result = FOE_INCOMPLETE;

    // set the final counts
    *pSrcCount -= srcCount;
    *pDstCount = dstCount;

    return result;
}

foeResult foe_utf32_to_utf8(size_t *pSrcCount,
                            uint32_t const *pSrc,
                            size_t *pDstCount,
                            uint8_t *pDst) {
    size_t srcCount = *pSrcCount;
    size_t dstCount = 0;

    if (*pSrcCount == 0) {
        // nothing to process
        *pDstCount = 0;
        return FOE_SUCCESS;
    } else if (pDst && *pDstCount == 0) {
        // no room in destination
        *pSrcCount = 0;
        return FOE_INCOMPLETE;
    }

    foeMultiByteState state = {0};
    foeResult result;
    size_t uncommittedWriteSize = 0;
    uint8_t out;

    // process until input exhausted
    while (srcCount != 0) {
        result = foe_utf32_to_utf8_ch(*pSrc, &out, &state);

        if (result == FOE_ERROR_UTF_MALFORMED_DATA || result == FOE_ERROR_UTF_INVALID_CODEPOINT) {
            break;
        }

        if (result == FOE_INCOMPLETE) {
            // outputting multi-byte
            ++uncommittedWriteSize;
        } else {
            // FOE_SUCCESS
            --srcCount;
            ++pSrc;
            uncommittedWriteSize = 0;
        }

        ++dstCount;
        if (pDst) {
            *pDst = out;
            ++pDst;
            if (dstCount == *pDstCount)
                // destination buffer full
                break;
        }
    }

    if (result != FOE_SUCCESS) {
        // failed to process everything
        dstCount -= uncommittedWriteSize;
    }

    // set the final counts
    *pSrcCount -= srcCount;
    *pDstCount = dstCount;

    return result;
}

foeResult foe_utf32_to_utf16(size_t *pSrcCount,
                             uint32_t const *pSrc,
                             size_t *pDstCount,
                             uint16_t *pDst) {
    size_t srcCount = *pSrcCount;
    size_t dstCount = 0;

    if (*pSrcCount == 0) {
        // nothing to process
        *pDstCount = 0;
        return FOE_SUCCESS;
    } else if (pDst && *pDstCount == 0) {
        // no room in destination
        *pSrcCount = 0;
        return FOE_INCOMPLETE;
    }

    foeMultiByteState state = {0};
    foeResult result;
    size_t uncommittedWriteSize = 0;
    uint16_t out;

    // process until input exhausted
    while (srcCount != 0) {
        result = foe_utf32_to_utf16_ch(*pSrc, &out, &state);

        if (result == FOE_ERROR_UTF_MALFORMED_DATA || result == FOE_ERROR_UTF_INVALID_CODEPOINT) {
            break;
        }

        if (result == FOE_INCOMPLETE) {
            // outputting multi-byte
            ++uncommittedWriteSize;
        } else {
            // FOE_SUCCESS
            --srcCount;
            ++pSrc;
            uncommittedWriteSize = 0;
        }

        ++dstCount;
        if (pDst) {
            *pDst = out;
            ++pDst;
            if (dstCount == *pDstCount)
                // destination buffer full
                break;
        }
    }

    if (result != FOE_SUCCESS) {
        // failed to process everything
        dstCount -= uncommittedWriteSize;
    }

    // set the final counts
    *pSrcCount -= srcCount;
    *pDstCount = dstCount;

    return result;
}