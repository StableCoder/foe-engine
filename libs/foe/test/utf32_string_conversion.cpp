// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/utf_string_conversion.h>

#include <cstdint>
#include <string>

TEST_CASE("UTF-32 to UTF-8 - string conversion") {
    std::u32string src;
    size_t srcCount;
    std::u8string dst;
    size_t dstCount = 0;
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("ハロー・ワールド") {
            src = U"ハロー・ワールド";
            srcCount = src.size();

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 24);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 24);
            CHECK(dst == u8"ハロー・ワールド");
        }

        SECTION("zß水🍌🍌") {
            src = U"zß水🍌🍌";
            srcCount = src.size();

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 5);
            CHECK(dstCount == 14);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 5);
            CHECK(dstCount == 14);
            CHECK(dst == u8"zß水🍌🍌");
        }
    }

    SECTION("convert full dataset with nul characters") {
        src.resize(6);
        src[0] = U'\0';
        src[1] = U't';
        src[2] = U'e';
        src[3] = U's';
        src[4] = U't';
        src[5] = U'\0';
        srcCount = 6;
        dst.resize(128);
        std::fill(dst.begin(), dst.end(), UINT16_MAX);
        dstCount = dst.size();

        result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                   (uint8_t *)dst.data());

        CHECK(result == FOE_SUCCESS);
        CHECK(srcCount == 6);
        CHECK(dstCount == 6);
        CHECK(memcmp(dst.data(), u8"\0test\0", dstCount) == 0);
    }

    SECTION("zero-sized source leads to 'successful' zero-sized conversion") {
        srcCount = 0;
        dst.resize(128);
        dst[0] = UINT8_MAX;
        dstCount = 128;

        SECTION("with no buffers") {
            result = foe_utf32_to_utf8(&srcCount, nullptr, &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only src buffer") {
            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only dst buffer") {
            result = foe_utf32_to_utf8(&srcCount, nullptr, &dstCount, (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with both src & dst buffers") {
            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("zero-sized destination buffer") {
        SECTION("with zero-sized source succeeds") {
            src = U"";
            srcCount = 0;
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with non-zero source returns incomplete") {
            src = U"z";
            srcCount = src.size();
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("with multi-byte codepoint insufficient destination room returns incomplete") {
        SECTION("starting with an error means no data converted") {
            src = U"🍌";
            srcCount = src.size();
            dst.resize(1);
            dstCount = 1;

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("having an issue partially through a conversion returns what could be successfully "
                "converted") {
            src = U"test🍌";
            srcCount = src.size();
            dst.resize(5);
            dstCount = 5;

            result = foe_utf32_to_utf8(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 4);
            CHECK(dstCount == 4);
            dst.resize(dstCount);
            CHECK(dst == u8"test");
        }
    }

    SECTION("check codepoint ranges") {
        dst.resize(128);

        SECTION("valid range 0x00 - 0x7F (1-byte)") {
            for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 1);
            }
        }
        SECTION("valid range 0x0080 - 0x07FF (2-byte)") {
            for (uint32_t codepoint = 0x0080; codepoint <= 0x07FF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 2);
            }
        }
        SECTION("valid range 0x0800 - 0xD7FF (3-byte)") {
            for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 3);
            }
        }
        SECTION("invalid range 0xD800 - 0xDFFF") {
            for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF (3-bytes)") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 3);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (4-bytes)") {
            for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 4);
            }
        }
        SECTION("invalid range 0x110000+") {
            uint32_t codepoint = 0x110000;
            srcCount = 1;
            dstCount = dst.size();

            result = foe_utf32_to_utf8(&srcCount, &codepoint, &dstCount, (uint8_t *)dst.data());

            CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }
}

TEST_CASE("UTF-32 to UTF-16 - string conversion") {
    std::u32string src;
    size_t srcCount;
    std::u16string dst;
    size_t dstCount = 0;
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("ハロー・ワールド") {
            src = U"ハロー・ワールド";
            srcCount = src.size();

            result =
                foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 8);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                        (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 8);
            CHECK(dst == u"ハロー・ワールド");
        }

        SECTION("zß水🍌🍌") {
            src = U"zß水🍌🍌";
            srcCount = src.size();

            result =
                foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 5);
            CHECK(dstCount == 7);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                        (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 5);
            CHECK(dstCount == 7);
            CHECK(dst == u"zß水🍌🍌");
        }
    }

    SECTION("convert full dataset with nul characters") {
        src.resize(6);
        src[0] = U'\0';
        src[1] = U't';
        src[2] = U'e';
        src[3] = U's';
        src[4] = U't';
        src[5] = U'\0';
        srcCount = 6;
        dst.resize(128);
        std::fill(dst.begin(), dst.end(), UINT16_MAX);
        dstCount = dst.size();

        result = foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                    (uint16_t *)dst.data());

        CHECK(result == FOE_SUCCESS);
        CHECK(srcCount == 6);
        CHECK(dstCount == 6);
        CHECK(memcmp(dst.data(), u"\0test\0", dstCount) == 0);
    }

    SECTION("zero-sized source leads to 'successful' zero-sized conversion") {
        srcCount = 0;
        dst.resize(128);
        dst[0] = UINT16_MAX;
        dstCount = 128;

        SECTION("with no buffers") {
            result = foe_utf32_to_utf16(&srcCount, nullptr, &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only src buffer") {
            result =
                foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only dst buffer") {
            result = foe_utf32_to_utf16(&srcCount, nullptr, &dstCount, (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with both src & dst buffers") {
            result = foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                        (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("zero-sized destination buffer") {
        SECTION("with zero-sized source succeeds") {
            src = U"";
            srcCount = 0;
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                        (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with non-zero source returns incomplete") {
            src = U"z";
            srcCount = src.size();
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf32_to_utf16(&srcCount, (uint32_t const *)src.data(), &dstCount,
                                        (uint16_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("check codepoint ranges") {
        dst.resize(128);

        SECTION("valid range 0x00 - 0xD7FF") {
            for (uint32_t codepoint = 0x00; codepoint < 0xD7FF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result =
                    foe_utf32_to_utf16(&srcCount, &codepoint, &dstCount, (uint16_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 1);
            }
        }
        SECTION("invalid range 0xD800 - 0xDFFF") {
            for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result =
                    foe_utf32_to_utf16(&srcCount, &codepoint, &dstCount, (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result =
                    foe_utf32_to_utf16(&srcCount, &codepoint, &dstCount, (uint16_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 1);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (surrogates)") {
            for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                srcCount = 1;
                dstCount = dst.size();

                result =
                    foe_utf32_to_utf16(&srcCount, &codepoint, &dstCount, (uint16_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 2);
            }
        }
        SECTION("invalid range 0x110000+") {
            uint32_t codepoint = 0x110000;
            srcCount = 1;
            dstCount = dst.size();

            result = foe_utf32_to_utf16(&srcCount, &codepoint, &dstCount, (uint16_t *)dst.data());

            CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }
}