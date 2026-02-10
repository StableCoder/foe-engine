// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

// NOTE: This is a private header as the API doesn't feel polished enough to be public

#ifndef FOE_UTF_CHARACTER_CONVERSION_H
#define FOE_UTF_CHARACTER_CONVERSION_H

#include <foe/export.h>
#include <foe/result.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum foeMultiByteProcessingFlagBits {
    PROCESSING_UTF8_TO_UTF16 = 0x00000001,
    PROCESSING_UTF8_TO_UTF32 = 0x00000002,
    PROCESSING_UTF16_TO_UTF8 = 0x00000004,
    PROCESSING_UTF16_TO_UTF32 = 0x00000010,
    PROCESSING_UTF32_TO_UTF8 = 0x00000020,
    PROCESSING_UTF32_TO_UTF16 = 0x00000040,
    AWAITING_1_POINTS = 0x00000100,
    AWAITING_2_POINTS = 0x00000400,
    AWAITING_3_POINTS = 0x00000800,
    PROCESSING_UTF8_4_POINT = 0x00001000,
    PROCESSING_UTF8_3_POINT = 0x00002000,
    PROCESSING_UTF8_2_POINT = 0x00004000,
    PROCESSING_UTF16_SURROGATE = 0x00008000,
    OUTPUTTING_UTF8_3_POINTS = 0x00010000,
    OUTPUTTING_UTF8_2_POINTS = 0x00020000,
    OUTPUTTING_UTF8_1_POINTS = 0x00040000,
    OUTPUTTING_UTF16_SURROGATE = 0x00080000,
};
typedef uint32_t foeMultiByteProcessingFlags;

typedef struct foeMultiByteState {
    foeMultiByteProcessingFlags state;
    uint32_t fullCodepoint;
} foeMultiByteState;

foeResult foe_utf8_to_utf16_ch(uint8_t src, uint16_t *pDst, foeMultiByteState *pState);

foeResult foe_utf8_to_utf32_ch(uint8_t src, uint32_t *pDst, foeMultiByteState *pState);

foeResult foe_utf16_to_utf8_ch(uint16_t src, uint8_t *pDst, foeMultiByteState *pState);

foeResult foe_utf16_to_utf32_ch(uint16_t src, uint32_t *pDst, foeMultiByteState *pState);

foeResult foe_utf32_to_utf8_ch(uint32_t src, uint8_t *pDst, foeMultiByteState *pState);

foeResult foe_utf32_to_utf16_ch(uint32_t src, uint16_t *pDst, foeMultiByteState *pState);

#ifdef __cplusplus
}
#endif

#endif // FOE_UTF_CHARACTER_CONVERSION_H