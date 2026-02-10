// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>

#include "utf_character_conversion.h"

TEST_CASE("UTF character conversion - changing between functions when incomplete") {
    uint8_t utf8 = u8"🍌"[0];
    uint16_t utf16 = u"🍌"[0];
    uint32_t utf32 = U"🍌"[0];
    foeResult result;
    foeMultiByteState state = {0};

    SECTION("start with UTF-8 to UTF-16") {
        result = foe_utf8_to_utf16_ch(utf8, &utf16, &state);
        CHECK(result == FOE_AWAITING_INPUT);

        SECTION("change to UTF-8 to UTF-32") {
            result = foe_utf8_to_utf32_ch(utf8, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-8") {
            result = foe_utf16_to_utf8_ch(utf16, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-32") {
            result = foe_utf16_to_utf32_ch(utf16, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-8") {
            result = foe_utf32_to_utf8_ch(utf32, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-16") {
            result = foe_utf32_to_utf16_ch(utf32, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
    }
    SECTION("start with UTF-8 to UTF-32") {
        result = foe_utf8_to_utf32_ch(utf8, &utf32, &state);
        CHECK(result == FOE_AWAITING_INPUT);

        SECTION("change to UTF-8 to UTF-16") {
            result = foe_utf8_to_utf16_ch(utf8, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-8") {
            result = foe_utf16_to_utf8_ch(utf16, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-32") {
            result = foe_utf16_to_utf32_ch(utf16, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-8") {
            result = foe_utf32_to_utf8_ch(utf32, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-16") {
            result = foe_utf32_to_utf16_ch(utf32, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
    }
    SECTION("start with UTF-16 to UTF-8") {
        result = foe_utf16_to_utf8_ch(utf16, &utf8, &state);
        CHECK(result == FOE_AWAITING_INPUT);

        SECTION("change to UTF-8 to UTF-16") {
            result = foe_utf8_to_utf16_ch(utf8, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-8 to UTF-32") {
            result = foe_utf8_to_utf32_ch(utf8, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-32") {
            result = foe_utf16_to_utf32_ch(utf16, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-8") {
            result = foe_utf32_to_utf8_ch(utf32, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-16") {
            result = foe_utf32_to_utf16_ch(utf32, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
    }
    SECTION("start with UTF-16 to UTF-32") {
        result = foe_utf16_to_utf32_ch(utf16, &utf32, &state);
        CHECK(result == FOE_AWAITING_INPUT);

        SECTION("change to UTF-8 to UTF-16") {
            result = foe_utf8_to_utf16_ch(utf8, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-8 to UTF-32") {
            result = foe_utf8_to_utf32_ch(utf8, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-8") {
            result = foe_utf16_to_utf8_ch(utf16, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-8") {
            result = foe_utf32_to_utf8_ch(utf32, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-16") {
            result = foe_utf32_to_utf16_ch(utf32, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
    }
    SECTION("start with UTF-32 to UTF-8") {
        result = foe_utf32_to_utf8_ch(utf32, &utf8, &state);
        CHECK(result == FOE_INCOMPLETE);

        SECTION("change to UTF-8 to UTF-16") {
            result = foe_utf8_to_utf16_ch(utf8, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-8 to UTF-32") {
            result = foe_utf8_to_utf32_ch(utf8, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-8") {
            result = foe_utf16_to_utf8_ch(utf16, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-32") {
            result = foe_utf16_to_utf32_ch(utf16, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-16") {
            result = foe_utf32_to_utf16_ch(utf32, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
    }
    SECTION("start with UTF-32 to UTF-16") {
        result = foe_utf32_to_utf16_ch(utf32, &utf16, &state);
        CHECK(result == FOE_INCOMPLETE);

        SECTION("change to UTF-8 to UTF-16") {
            result = foe_utf8_to_utf16_ch(utf8, &utf16, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-8 to UTF-32") {
            result = foe_utf8_to_utf32_ch(utf8, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-8") {
            result = foe_utf16_to_utf8_ch(utf16, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-16 to UTF-32") {
            result = foe_utf16_to_utf32_ch(utf16, &utf32, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
        SECTION("change to UTF-32 to UTF-8") {
            result = foe_utf32_to_utf8_ch(utf32, &utf8, &state);
            CHECK(result == FOE_ERROR_UTF_INVALID_STATE);
        }
    }
}