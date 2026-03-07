// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>

#include "utf_character_conversion.h"

#include <array>
#include <cstdint>

TEST_CASE("UTF-16 to UTF-8 - character conversion") {
    foeMultiByteState state = {};
    foeResult result;

    SECTION("check codepoint ranges") {
        uint8_t dst;

        SECTION("valid range 0x00 - 0x7F (1-byte)") {
            for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                uint16_t src = codepoint;
                state = {0};

                // intake codepoint, output only byte
                result = foe_utf16_to_utf8_ch(src, &dst, &state);

                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x080 - 0x07FF (2-byte)") {
            for (uint32_t codepoint = 0x080; codepoint <= 0x07FF; ++codepoint) {
                uint16_t src = codepoint;
                state = {0};

                // intake codepoint, output first byte
                result = foe_utf16_to_utf8_ch(src, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                // output second byte
                result = foe_utf16_to_utf8_ch(src, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF (3-byte)") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                uint16_t src = codepoint;
                state = {0};

                // intake codepoint, output first byte
                result = foe_utf16_to_utf8_ch(src, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                // output second byte
                result = foe_utf16_to_utf8_ch(src, &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                // output third byte
                result = foe_utf16_to_utf8_ch(src, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (4-byte)") {
            for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                uint32_t adjustedCodepoint = codepoint - 0x10000;
                std::array<uint16_t, 2> src;
                src[0] = 0xD800 | ((adjustedCodepoint >> 10) & 0x03FF);
                src[1] = 0xDC00 | (adjustedCodepoint & 0x03FF);
                state = {0};

                // intake first surrogate
                result = foe_utf16_to_utf8_ch(src[0], &dst, &state);
                CHECK(result == FOE_AWAITING_INPUT);

                // intake low surrogate, output first byte
                result = foe_utf16_to_utf8_ch(src[1], &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                // output second byte
                result = foe_utf16_to_utf8_ch(src[1], &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                // output third byte
                result = foe_utf16_to_utf8_ch(src[1], &dst, &state);
                CHECK(result == FOE_INCOMPLETE);

                // output fourth byte
                result = foe_utf16_to_utf8_ch(src[1], &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
    }
}

TEST_CASE("UTF-16 to UTF-32 - character conversion") {
    foeMultiByteState state = {};
    foeResult result;

    SECTION("check codepoint ranges") {
        uint32_t dst;

        SECTION("valid range 0x0000 - 0xD7FF") {
            for (uint32_t codepoint = 0x0000; codepoint <= 0xD7FF; ++codepoint) {
                uint16_t src = codepoint;
                state = {0};

                // intake codepoint and output
                result = foe_utf16_to_utf32_ch(src, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF") {
            for (uint32_t codepoint = 0x080; codepoint <= 0x07FF; ++codepoint) {
                uint16_t src = codepoint;
                state = {0};

                // intake codepoint and output
                result = foe_utf16_to_utf32_ch(src, &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (surrogate input)") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                uint32_t adjustedCodepoint = codepoint - 0x10000;
                std::array<uint16_t, 2> src;
                src[0] = 0xD800 | ((adjustedCodepoint >> 10) & 0x03FF);
                src[1] = 0xDC00 | (adjustedCodepoint & 0x03FF);
                state = {0};

                // intake high surrogate
                result = foe_utf16_to_utf32_ch(src[0], &dst, &state);
                CHECK(result == FOE_AWAITING_INPUT);

                // intake low surrogate and output
                result = foe_utf16_to_utf32_ch(src[1], &dst, &state);
                CHECK(result == FOE_SUCCESS);
            }
        }
    }
}