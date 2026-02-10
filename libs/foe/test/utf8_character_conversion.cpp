// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>

#include "utf_character_conversion.h"

#include <array>
#include <cstdint>

TEST_CASE("UTF-8 to UTF-16 - character conversion") {
    std::u8string src;
    std::u16string dst;
    foeMultiByteState state = {};
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("1-byte z") {
            src = u8"z";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf16_ch(src[0], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == u"z");
        }
        SECTION("2-byte ß") {
            src = u8"ß";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf16_ch(src[0], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[1], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == u"ß");
        }
        SECTION("3-byte ハ") {
            src = u8"ハ";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf16_ch(src[0], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[1], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[2], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == u"ハ");
        }
        SECTION("4-byte 🍌") {
            src = u8"🍌";
            dst.resize(2);
            dst[0] = 0;
            dst[1] = 0;

            result = foe_utf8_to_utf16_ch(src[0], (uint16_t *)dst.data(), &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[1], (uint16_t *)dst.data(), &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[2], (uint16_t *)dst.data(), &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[3], (uint16_t *)dst.data(), &state);
            CHECK(result == FOE_INCOMPLETE);

            result = foe_utf8_to_utf16_ch(src[3], (uint16_t *)dst.data() + 1, &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == u"🍌");
        }

        SECTION("convert nul character") {
            src.resize(1);
            dst.resize(1);
            std::fill(dst.begin(), dst.end(), UINT16_MAX);

            result = foe_utf8_to_utf16_ch(src[0], (uint16_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(memcmp(dst.data(), u"\0", sizeof(uint16_t)) == 0);
        }
    }

    SECTION("malformed multi-byte input failures") {
        uint16_t dst;

        SECTION("start with second byte (continuation) of '🍌'") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("start with third byte (continuation) of '🍌'") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("start with fourth byte (continuation) of '🍌'") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[3], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("start '🍌', but change to 'z' after 1 bytes") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start '🍌', but change to 'z' after 2 bytes") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);
            result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start '🍌', but change to 'z' after 3 bytes") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);
            result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);
            result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start with first byte of '🍌', then start another multi-byte input") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start with first byte of '🍌', then have a regular single-byte input") {
            src = u8"🍌";

            result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf16_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
    }

    SECTION("check codepoint ranges") {
        uint16_t dst;

        SECTION("invalid start byte of 0x80") {
            uint8_t src = 0x80;

            result = foe_utf8_to_utf16_ch(src, &dst, &state);

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("invalid start byte of 0xFF") {
            uint8_t src = 0xFF;

            result = foe_utf8_to_utf16_ch(src, &dst, &state);

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("codepoint ranges") {
            SECTION("1-byte") {
                SECTION("valid range 0x00 - 0x7F") {
                    uint8_t src;

                    for (uint32_t codepoint = 0; codepoint <= 0x7F; ++codepoint) {
                        src = codepoint;
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src, &dst, &state);

                        CHECK(result == FOE_SUCCESS);
                    }
                }
            }

            SECTION("2-byte") {
                std::array<uint8_t, 2> src;

                SECTION("invalid range 0x00 - 0x7F") {
                    for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("valid range 0x0080 - 0x07FF") {
                    for (uint32_t codepoint = 0x0080; codepoint <= 0x07FF; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
            }

            SECTION("3-byte") {
                std::array<uint8_t, 3> src;

                SECTION("invalid range 0x0000 - 0x0800") {
                    for (uint32_t codepoint = 0; codepoint < 0x0800; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("valid range 0x0800 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                    }
                }
                SECTION("valid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
            }

            SECTION("4-byte") {
                std::array<uint8_t, 4> src;

                SECTION("invalid range 0x0000 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0000; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[3], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[3], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                    }
                }
                SECTION("invalid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[3], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("valid range 0x010000 - 0x10FFFF") {
                    for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf16_ch(src[3], &dst, &state);
                        CHECK(result == FOE_INCOMPLETE);
                    }
                }
                SECTION("invalid range 0x110000+") {
                    uint32_t codepoint = 0x110000;
                    src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                    src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                    src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                    src[3] = 0x80 | (codepoint & 0x3F);
                    state = {0};

                    result = foe_utf8_to_utf16_ch(src[0], &dst, &state);
                    CHECK(result == FOE_AWAITING_INPUT);

                    result = foe_utf8_to_utf16_ch(src[1], &dst, &state);
                    CHECK(result == FOE_AWAITING_INPUT);

                    result = foe_utf8_to_utf16_ch(src[2], &dst, &state);
                    CHECK(result == FOE_AWAITING_INPUT);

                    result = foe_utf8_to_utf16_ch(src[3], &dst, &state);
                    CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                }
            }
        }
    }
}

TEST_CASE("UTF-8 to UTF-32 - character conversion") {
    std::u8string src;
    size_t srcCount = 0;
    std::u32string dst;
    size_t dstCount = 0;
    foeMultiByteState state = {};
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("1-byte z") {
            src = u8"z";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf32_ch(src[0], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == U"z");
        }
        SECTION("2-byte ß") {
            src = u8"ß";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf32_ch(src[0], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[1], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == U"ß");
        }
        SECTION("3-byte ハ") {
            src = u8"ハ";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf32_ch(src[0], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[1], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[2], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == U"ハ");
        }
        SECTION("4-byte 🍌") {
            src = u8"🍌";
            dst.resize(1);
            dst[0] = 0;

            result = foe_utf8_to_utf32_ch(src[0], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[1], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[2], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[3], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(dst == U"🍌");
        }

        SECTION("convert nul character") {
            src.resize(1);
            srcCount = src.size();
            dst.resize(1);
            std::fill(dst.begin(), dst.end(), UINT16_MAX);
            dstCount = dst.size();

            result = foe_utf8_to_utf32_ch(src[0], (uint32_t *)dst.data(), &state);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 1);
            CHECK(dstCount == 1);
            CHECK(memcmp(dst.data(), u"\0", dstCount) == 0);
        }
    }

    SECTION("malformed multi-byte input failures") {
        uint32_t dst;

        SECTION("start with second byte (continuation) of '🍌'") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("start with third byte (continuation) of '🍌'") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("start with fourth byte (continuation) of '🍌'") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[3], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("start '🍌', but change to 'z' after 1 bytes") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start '🍌', but change to 'z' after 2 bytes") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);
            result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start '🍌', but change to 'z' after 3 bytes") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);
            result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);
            result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start with first byte of '🍌', then start another multi-byte input") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
        SECTION("start with first byte of '🍌', then have a regular single-byte input") {
            src = u8"🍌";

            result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
            CHECK(result == FOE_AWAITING_INPUT);

            result = foe_utf8_to_utf32_ch('z', &dst, &state);
            CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
        }
    }

    SECTION("check codepoint ranges") {
        uint32_t dst;

        SECTION("invalid start byte of 0x80") {
            uint8_t src = 0x80;

            result = foe_utf8_to_utf32_ch(src, &dst, &state);

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("invalid start byte of 0xFF") {
            uint8_t src = 0xFF;

            result = foe_utf8_to_utf32_ch(src, &dst, &state);

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
        }
        SECTION("codepoint ranges") {
            SECTION("1-byte") {
                SECTION("valid range 0x00 - 0x7F") {
                    uint8_t src;

                    for (uint32_t codepoint = 0; codepoint <= 0x7F; ++codepoint) {
                        src = codepoint;
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src, &dst, &state);

                        CHECK(result == FOE_SUCCESS);
                    }
                }
            }

            SECTION("2-byte") {
                std::array<uint8_t, 2> src;

                SECTION("invalid range 0x00 - 0x7F") {
                    for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("valid range 0x0080 - 0x07FF") {
                    for (uint32_t codepoint = 0x0080; codepoint <= 0x07FF; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
            }

            SECTION("3-byte") {
                std::array<uint8_t, 3> src;

                SECTION("invalid range 0x0000 - 0x0800") {
                    for (uint32_t codepoint = 0; codepoint < 0x0800; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("valid range 0x0800 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                    }
                }
                SECTION("valid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
            }

            SECTION("4-byte") {
                std::array<uint8_t, 4> src;

                SECTION("invalid range 0x0000 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0000; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[3], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[3], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                    }
                }
                SECTION("invalid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[3], &dst, &state);
                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                    }
                }
                SECTION("valid range 0x010000 - 0x10FFFF") {
                    for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);
                        state = {0};

                        result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                        CHECK(result == FOE_AWAITING_INPUT);

                        result = foe_utf8_to_utf32_ch(src[3], &dst, &state);
                        CHECK(result == FOE_SUCCESS);
                    }
                }
                SECTION("invalid range 0x110000+") {
                    uint32_t codepoint = 0x110000;
                    src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                    src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                    src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                    src[3] = 0x80 | (codepoint & 0x3F);
                    state = {0};

                    result = foe_utf8_to_utf32_ch(src[0], &dst, &state);
                    CHECK(result == FOE_AWAITING_INPUT);

                    result = foe_utf8_to_utf32_ch(src[1], &dst, &state);
                    CHECK(result == FOE_AWAITING_INPUT);

                    result = foe_utf8_to_utf32_ch(src[2], &dst, &state);
                    CHECK(result == FOE_AWAITING_INPUT);

                    result = foe_utf8_to_utf32_ch(src[3], &dst, &state);
                    CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                }
            }
        }
    }
}