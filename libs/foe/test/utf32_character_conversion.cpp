// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>

#include "utf_character_conversion.h"

#include <array>
#include <cstdint>

TEST_CASE("UTF-32 to UTF-8 - character conversion") {
    foeMultiByteState state = {};
    foeResult result;

    SECTION("check codepoint ranges") {
        uint8_t dst;

        SECTION("valid range 0x00 - 0x7F (1-byte)") {
            for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x0080 - 0x07FF (2-byte)") {
            for (uint32_t codepoint = 0x0080; codepoint <= 0x07FF; ++codepoint) {
                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x0800 - 0xD7FF (3-byte)") {
            for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("invalid range 0xD800 - 0xDFFF") {
            for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF (3-byte)") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (4-byte)") {
            for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("invalid range 0x110000+") {
            uint32_t codepoint = 0x110000;
            result = foe_utf32_to_utf8_ch(codepoint, &dst, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
        }
    }
}

TEST_CASE("UTF-32 to UTF16 - character conversion") {
    foeMultiByteState state = {};
    foeResult result;

    SECTION("check codepoint ranges") {
        uint16_t dst;

        SECTION("valid range 0x00 - 0xD7FF") {
            for (uint32_t codepoint = 0x00; codepoint <= 0xD7FF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf16_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("invalid range 0xD800 - 0xDFFF") {
            for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf16_ch(codepoint, &dst, &state);
                CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF") {
            for (uint32_t codepoint = 0xE000; codepoint < 0xFFFF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf16_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (surrogates)") {
            for (uint32_t codepoint = 0x010000; codepoint < 0x10FFFF; ++codepoint) {
                state = {0};

                result = foe_utf32_to_utf16_ch(codepoint, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                result = foe_utf32_to_utf16_ch(codepoint, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("invalid range 0x110000+") {
            uint32_t codepoint = 0x110000;
            result = foe_utf32_to_utf16_ch(codepoint, &dst, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
        }
    }
}