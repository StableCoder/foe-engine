// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_UTF_STRING_CONVERSION_H
#define FOE_UTF_STRING_CONVERSION_H

#include <foe/export.h>
#include <foe/result.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

FOE_EXPORT
foeResult foe_utf8_to_utf16(size_t *pSrcCount,
                            uint8_t const *pSrc,
                            size_t *pDstCount,
                            uint16_t *pDst);

FOE_EXPORT
foeResult foe_utf8_to_utf32(size_t *pSrcCount,
                            uint8_t const *pSrc,
                            size_t *pDstCount,
                            uint32_t *pDst);

FOE_EXPORT
foeResult foe_utf16_to_utf8(size_t *pSrcCount,
                            uint16_t const *pSrc,
                            size_t *pDstCount,
                            uint8_t *pDst);

FOE_EXPORT
foeResult foe_utf16_to_utf32(size_t *pSrcCount,
                             uint16_t const *pSrc,
                             size_t *pDstCount,
                             uint32_t *pDst);

FOE_EXPORT
foeResult foe_utf32_to_utf8(size_t *pSrcCount,
                            uint32_t const *pSrc,
                            size_t *pDstCount,
                            uint8_t *pDst);

FOE_EXPORT
foeResult foe_utf32_to_utf16(size_t *pSrcCount,
                             uint32_t const *pSrc,
                             size_t *pDstCount,
                             uint16_t *pDst);

#ifdef __cplusplus
}
#endif

#endif // FOE_UTF_STRING_CONVERSION_H