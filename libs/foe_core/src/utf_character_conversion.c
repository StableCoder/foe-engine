// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include "utf_character_conversion.h"

#include <assert.h>
#include <string.h>

foeResult foe_utf8_to_utf16_ch(uint8_t src, uint16_t *pDst, foeMultiByteState *pState) {
    if (pState->state == 0x0) {
        // determine if multi-byte or not
        if ((src & 0x80) == 0) {
            // just a single byte, output immediately
            *pDst = src;
            return FOE_SUCCESS;

        } else if ((src & 0xF8) == 0xF0) {
            // start of a 4 point
            pState->state = PROCESSING_UTF8_TO_UTF16 | PROCESSING_UTF8_4_POINT | AWAITING_3_POINTS;
            pState->fullCodepoint = (src & 0x07) << 18;
            return FOE_AWAITING_INPUT;

        } else if ((src & 0xF0) == 0xE0) {
            // start of a 3 point
            pState->state = PROCESSING_UTF8_TO_UTF16 | PROCESSING_UTF8_3_POINT | AWAITING_2_POINTS;
            pState->fullCodepoint = (src & 0x0F) << 12;
            return FOE_AWAITING_INPUT;

        } else if ((src & 0xE0) == 0xC0) {
            // start of a 2 point
            pState->state = PROCESSING_UTF8_TO_UTF16 | PROCESSING_UTF8_2_POINT | AWAITING_1_POINTS;
            pState->fullCodepoint = (src & 0x1F) << 6;
            return FOE_AWAITING_INPUT;

        } else {
            // unknown/unexpected input
            return FOE_ERROR_UTF_MALFORMED_DATA;
        }
    } else if ((pState->state & PROCESSING_UTF8_TO_UTF16) == 0) {
        return FOE_ERROR_UTF_INVALID_STATE;

    } else if ((pState->state & AWAITING_1_POINTS) == AWAITING_1_POINTS) {
        if ((src & 0xC0) != 0x80) {
            // expected continuation, got something else
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_MB_INCOMPLETE;
        }

        pState->fullCodepoint |= (src & 0x3F);

        // check the codepoint for validity
        if (pState->fullCodepoint >= 0xD800 && pState->fullCodepoint <= 0xDFFF) {
            // invalid range for UTF16 surrogate pairs
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_INVALID_CODEPOINT;

        } else if (pState->fullCodepoint > 0x10FFFF) {
            // invalid range
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        }

        if ((pState->state & PROCESSING_UTF8_2_POINT) == PROCESSING_UTF8_2_POINT) {
            if (pState->fullCodepoint < 0x0080 || pState->fullCodepoint > 0x07FF) {
                memset(pState, 0, sizeof(foeMultiByteState));
                return FOE_ERROR_UTF_MALFORMED_DATA;
            }
        } else if ((pState->state & PROCESSING_UTF8_3_POINT) == PROCESSING_UTF8_3_POINT) {
            if (pState->fullCodepoint < 0x0800 || pState->fullCodepoint > 0xFFFF) {
                memset(pState, 0, sizeof(foeMultiByteState));
                return FOE_ERROR_UTF_MALFORMED_DATA;
            }
        } else if ((pState->state & PROCESSING_UTF8_4_POINT) == PROCESSING_UTF8_4_POINT) {
            if (pState->fullCodepoint < 0x010000) {
                memset(pState, 0, sizeof(foeMultiByteState));
                return FOE_ERROR_UTF_MALFORMED_DATA;
            }
        }

        if (pState->fullCodepoint <= 0xFFFF) {
            // can be output as a single UTF-16 codepoint
            *pDst = pState->fullCodepoint;
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_SUCCESS;
        }

        // output high surrogate
        pState->fullCodepoint -= 0x10000;
        *pDst = 0xD800 + ((pState->fullCodepoint >> 10) & 0x3FF);
        pState->state = PROCESSING_UTF8_TO_UTF16 | OUTPUTTING_UTF16_SURROGATE;
        return FOE_INCOMPLETE;

    } else if ((pState->state & OUTPUTTING_UTF16_SURROGATE) == OUTPUTTING_UTF16_SURROGATE) {
        *pDst = 0xDC00 + (pState->fullCodepoint & 0x3FF);
        memset(pState, 0, sizeof(foeMultiByteState));
        return FOE_SUCCESS;

    } else if ((pState->state & AWAITING_2_POINTS) == AWAITING_2_POINTS) {
        if ((src & 0xC0) != 0x80) {
            // expected continuation, got something else
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_MB_INCOMPLETE;
        }

        pState->state &= ~AWAITING_2_POINTS;
        pState->state |= AWAITING_1_POINTS;
        pState->fullCodepoint |= (src & 0x3F) << 6;
        return FOE_AWAITING_INPUT;
    }

    assert((pState->state & AWAITING_3_POINTS) == AWAITING_3_POINTS);
    if ((src & 0xC0) != 0x80) {
        // expected continuation, got something else
        memset(pState, 0, sizeof(foeMultiByteState));
        return FOE_ERROR_UTF_MB_INCOMPLETE;
    }

    pState->state &= ~AWAITING_3_POINTS;
    pState->state |= AWAITING_2_POINTS;
    pState->fullCodepoint |= (src & 0x3F) << 12;
    return FOE_AWAITING_INPUT;
}

foeResult foe_utf8_to_utf32_ch(uint8_t src, uint32_t *pDst, foeMultiByteState *pState) {
    if (pState->state == 0x0) {
        // determine if multi-byte or not
        if ((src & 0x80) == 0) {
            // just a single byte, output immediately
            *pDst = src;
            return FOE_SUCCESS;

        } else if ((src & 0xF8) == 0xF0) {
            // start of a 4 point
            pState->state = PROCESSING_UTF8_TO_UTF32 | PROCESSING_UTF8_4_POINT | AWAITING_3_POINTS;
            pState->fullCodepoint = (src & 0x07) << 18;
            return FOE_AWAITING_INPUT;

        } else if ((src & 0xF0) == 0xE0) {
            // start of a 3 point
            pState->state = PROCESSING_UTF8_TO_UTF32 | PROCESSING_UTF8_3_POINT | AWAITING_2_POINTS;
            pState->fullCodepoint = (src & 0x0F) << 12;
            return FOE_AWAITING_INPUT;

        } else if ((src & 0xE0) == 0xC0) {
            // start of a 2 point
            pState->state = PROCESSING_UTF8_TO_UTF32 | PROCESSING_UTF8_2_POINT | AWAITING_1_POINTS;
            pState->fullCodepoint = (src & 0x1F) << 6;
            return FOE_AWAITING_INPUT;

        } else {
            // unknown/unexpected input
            return FOE_ERROR_UTF_MALFORMED_DATA;
        }
    } else if ((pState->state & PROCESSING_UTF8_TO_UTF32) == 0) {
        return FOE_ERROR_UTF_INVALID_STATE;

    } else if ((pState->state & AWAITING_1_POINTS) == AWAITING_1_POINTS) {
        if ((src & 0xC0) != 0x80) {
            // expected continuation, got something else
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_MB_INCOMPLETE;
        }

        pState->fullCodepoint |= (src & 0x3F);

        // check the codepoint for validity
        if (pState->fullCodepoint >= 0xD800 && pState->fullCodepoint <= 0xDFFF) {
            // invalid range for UTF16 surrogate pairs
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        } else if (pState->fullCodepoint > 0x10FFFF) {
            // invalid range
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        }

        if ((pState->state & PROCESSING_UTF8_2_POINT) == PROCESSING_UTF8_2_POINT) {
            if (pState->fullCodepoint < 0x0080 || pState->fullCodepoint > 0x07FF) {
                memset(pState, 0, sizeof(foeMultiByteState));
                return FOE_ERROR_UTF_MALFORMED_DATA;
            }
        } else if ((pState->state & PROCESSING_UTF8_3_POINT) == PROCESSING_UTF8_3_POINT) {
            if (pState->fullCodepoint < 0x0800 || pState->fullCodepoint > 0xFFFF) {
                memset(pState, 0, sizeof(foeMultiByteState));
                return FOE_ERROR_UTF_MALFORMED_DATA;
            }
        } else if ((pState->state & PROCESSING_UTF8_4_POINT) == PROCESSING_UTF8_4_POINT) {
            if (pState->fullCodepoint < 0x010000) {
                memset(pState, 0, sizeof(foeMultiByteState));
                return FOE_ERROR_UTF_MALFORMED_DATA;
            }
        }

        // output value
        *pDst = pState->fullCodepoint;
        memset(pState, 0, sizeof(foeMultiByteState));

        return FOE_SUCCESS;
    } else if ((pState->state & AWAITING_2_POINTS) == AWAITING_2_POINTS) {
        if ((src & 0xC0) != 0x80) {
            // expected continuation, got something else
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_MB_INCOMPLETE;
        }

        pState->state &= ~AWAITING_2_POINTS;
        pState->state |= AWAITING_1_POINTS;
        pState->fullCodepoint |= (src & 0x3F) << 6;
        return FOE_AWAITING_INPUT;
    }

    assert((pState->state & AWAITING_3_POINTS) == AWAITING_3_POINTS);
    if ((src & 0xC0) != 0x80) {
        // expected continuation, got something else
        memset(pState, 0, sizeof(foeMultiByteState));
        return FOE_ERROR_UTF_MB_INCOMPLETE;
    }

    pState->state &= ~AWAITING_3_POINTS;
    pState->state |= AWAITING_2_POINTS;
    pState->fullCodepoint |= (src & 0x3F) << 12;
    return FOE_AWAITING_INPUT;
}

foeResult foe_utf16_to_utf8_ch(uint16_t src, uint8_t *pDst, foeMultiByteState *pState) {
    if (pState->state == 0x0) {
        // single-byte return
        if (src <= 0x7F) {
            *pDst = (uint8_t)src;
            return FOE_SUCCESS;
        }

        if ((src & 0xFC00) == 0xD800) {
            // high surrogate
            pState->state = PROCESSING_UTF16_TO_UTF8 | PROCESSING_UTF16_SURROGATE;
            pState->fullCodepoint = (src - 0xD800) << 10;
            return FOE_AWAITING_INPUT;

        } else if ((src & 0xFC00) == 0xDC00) {
            // invalid starting with low surrogate
            return FOE_ERROR_UTF_MALFORMED_DATA;
        }

        // for 4-byte output, always need the surrogate
        assert(src <= 0xFFFF);

        // otherwise a non-surrogate multi-byte output
        pState->fullCodepoint = src;
        if (src <= 0x07FF) {
            pState->state = PROCESSING_UTF16_TO_UTF8 | OUTPUTTING_UTF8_1_POINTS;
            *pDst = 0xC0 | ((src >> 6) & 0x1F);
            return FOE_INCOMPLETE;
        }

        pState->state = PROCESSING_UTF16_TO_UTF8 | OUTPUTTING_UTF8_2_POINTS;
        *pDst = 0xE0 | ((src >> 12) & 0x0F);
        return FOE_INCOMPLETE;

    } else if ((pState->state & PROCESSING_UTF16_TO_UTF8) == 0) {
        return FOE_ERROR_UTF_INVALID_STATE;

    } else if ((pState->state & OUTPUTTING_UTF8_1_POINTS) == OUTPUTTING_UTF8_1_POINTS) {
        // output the last and reset
        *pDst = 0x80 | (pState->fullCodepoint & 0x3F);
        memset(pState, 0, sizeof(foeMultiByteState));
        return FOE_SUCCESS;

    } else if ((pState->state & PROCESSING_UTF16_SURROGATE) == PROCESSING_UTF16_SURROGATE) {
        if ((src & 0xFC00) != 0xDC00) {
            // low surrogate has specific starting bytes
            memset(pState, 0, sizeof(foeMultiByteState));
            return FOE_ERROR_UTF_MALFORMED_DATA;
        }

        pState->fullCodepoint |= (src - 0xDC00);
        pState->fullCodepoint += 0x10000;

        // output first of 4 bytes
        pState->state &= ~PROCESSING_UTF16_SURROGATE;
        pState->state |= OUTPUTTING_UTF8_3_POINTS;
        *pDst = 0xF0 | (pState->fullCodepoint >> 18) & 0x07;
        return FOE_INCOMPLETE;

    } else if ((pState->state & OUTPUTTING_UTF8_2_POINTS) == OUTPUTTING_UTF8_2_POINTS) {
        // output second to last
        *pDst = 0x80 | ((pState->fullCodepoint >> 6) & 0x3F);
        pState->state &= ~OUTPUTTING_UTF8_2_POINTS;
        pState->state |= OUTPUTTING_UTF8_1_POINTS;
        return FOE_INCOMPLETE;
    }

    assert((pState->state & OUTPUTTING_UTF8_3_POINTS) == OUTPUTTING_UTF8_3_POINTS);
    // output second to last
    *pDst = 0x80 | ((pState->fullCodepoint >> 12) & 0x3F);
    pState->state &= ~OUTPUTTING_UTF8_3_POINTS;
    pState->state |= OUTPUTTING_UTF8_2_POINTS;
    return FOE_INCOMPLETE;
}

foeResult foe_utf16_to_utf32_ch(uint16_t src, uint32_t *pDst, foeMultiByteState *pState) {
    if (pState->state == 0x0) {
        // single-byte return
        if (src <= 0x7F) {
            *pDst = (uint8_t)src;
            return FOE_SUCCESS;
        }

        if ((src & 0xFC00) == 0xD800) {
            // high surrogate
            pState->state = PROCESSING_UTF16_TO_UTF32 | PROCESSING_UTF16_SURROGATE;
            pState->fullCodepoint = (src - 0xD800) << 10;
            return FOE_AWAITING_INPUT;

        } else if ((src & 0xFC00) == 0xDC00) {
            // invalid starting with low surrogate
            return FOE_ERROR_UTF_MALFORMED_DATA;
        }

        *pDst = src;
        return FOE_SUCCESS;

    } else if ((pState->state & PROCESSING_UTF16_TO_UTF32) == 0) {
        return FOE_ERROR_UTF_INVALID_STATE;
    }

    assert((pState->state & PROCESSING_UTF16_SURROGATE) == PROCESSING_UTF16_SURROGATE);
    if ((src & 0xFC00) != 0xDC00) {
        // low surrogate has specific starting bytes
        memset(pState, 0, sizeof(foeMultiByteState));
        return FOE_ERROR_UTF_MALFORMED_DATA;
    }

    pState->fullCodepoint |= (src - 0xDC00);
    pState->fullCodepoint += 0x10000;

    // output
    *pDst = pState->fullCodepoint;
    memset(pState, 0, sizeof(foeMultiByteState));
    return FOE_SUCCESS;
}

foeResult foe_utf32_to_utf8_ch(uint32_t src, uint8_t *pDst, foeMultiByteState *pState) {
    if (pState->state == 0x0) {
        if (src >= 0xD800 && src <= 0xDFFF) {
            // reserved for UTF16 surrogates
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        } else if (src > 0x10FFFF) {
            // 0x10FFFF is the maximum valid value
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        }

        if (src <= 0x7F) {
            // output 1-byte
            *pDst = (uint8_t)src;
            return FOE_SUCCESS;
        }

        pState->fullCodepoint = src;
        if (src <= 0x07FF) {
            pState->state = PROCESSING_UTF32_TO_UTF8 | OUTPUTTING_UTF8_1_POINTS;
            *pDst = 0xC0 | ((src >> 6) & 0x1F);
            return FOE_INCOMPLETE;

        } else if (src <= 0xFFFF) {
            pState->state = PROCESSING_UTF32_TO_UTF8 | OUTPUTTING_UTF8_2_POINTS;
            *pDst = 0xE0 | ((src >> 12) & 0x0F);
            return FOE_INCOMPLETE;
        }

        pState->state = PROCESSING_UTF32_TO_UTF8 | OUTPUTTING_UTF8_3_POINTS;
        *pDst = 0xF0 | ((src >> 18) & 0x07);
        return FOE_INCOMPLETE;

    } else if ((pState->state & PROCESSING_UTF32_TO_UTF8) == 0) {
        return FOE_ERROR_UTF_INVALID_STATE;

    } else if ((pState->state & OUTPUTTING_UTF8_1_POINTS) == OUTPUTTING_UTF8_1_POINTS) {
        *pDst = 0x80 | (src & 0x3F);
        memset(pState, 0, sizeof(foeMultiByteState));
        return FOE_SUCCESS;

    } else if ((pState->state & OUTPUTTING_UTF8_2_POINTS) == OUTPUTTING_UTF8_2_POINTS) {
        pState->state &= ~OUTPUTTING_UTF8_2_POINTS;
        pState->state |= OUTPUTTING_UTF8_1_POINTS;
        *pDst = 0x80 | ((src >> 6) & 0x3F);
        return FOE_INCOMPLETE;
    }

    assert((pState->state & OUTPUTTING_UTF8_3_POINTS) == OUTPUTTING_UTF8_3_POINTS);
    pState->state &= ~OUTPUTTING_UTF8_3_POINTS;
    pState->state |= OUTPUTTING_UTF8_2_POINTS;
    *pDst = 0x80 | ((src >> 12) & 0x3F);
    return FOE_INCOMPLETE;
}

foeResult foe_utf32_to_utf16_ch(uint32_t src, uint16_t *pDst, foeMultiByteState *pState) {
    if (pState->state == 0x0) {
        if (src >= 0xD800 && src <= 0xDFFF) {
            // reserved for UTF16 surrogates
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        } else if (src > 0x10FFFF) {
            // 0x10FFFF is the maximum valid value
            return FOE_ERROR_UTF_INVALID_CODEPOINT;
        }

        if (src <= 0xFFFF) {
            // output 1-byte
            *pDst = (uint16_t)src;
            return FOE_SUCCESS;
        }

        // output high surrogate
        pState->state = PROCESSING_UTF32_TO_UTF16 | OUTPUTTING_UTF16_SURROGATE;
        pState->fullCodepoint = src - 0x10000;
        *pDst = 0xD800 + ((pState->fullCodepoint >> 10) & 0x3FF);
        return FOE_INCOMPLETE;

    } else if ((pState->state & PROCESSING_UTF32_TO_UTF16) == 0) {
        return FOE_ERROR_UTF_INVALID_STATE;
    }

    assert((pState->state & OUTPUTTING_UTF16_SURROGATE) == OUTPUTTING_UTF16_SURROGATE);
    *pDst = 0xDC00 + (pState->fullCodepoint & 0x3FF);
    memset(pState, 0, sizeof(foeMultiByteState));
    return FOE_SUCCESS;
}