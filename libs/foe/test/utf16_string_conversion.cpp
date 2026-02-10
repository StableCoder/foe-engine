// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/utf_string_conversion.h>

#include <array>
#include <cstdint>
#include <string>

TEST_CASE("UTF-16 to UTF-8 - string conversion") {
    std::u16string src;
    size_t srcCount;
    std::u8string dst;
    size_t dstCount = 0;
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("ハロー・ワールド") {
            src = u"ハロー・ワールド";
            srcCount = src.size();

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 24);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 24);
            CHECK(dst == u8"ハロー・ワールド");
        }
        SECTION("zß水🍌") {
            src = u"zß水🍌🍌";
            srcCount = src.size();

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 7);
            CHECK(dstCount == 14);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 7);
            CHECK(dstCount == 14);
            CHECK(dst == u8"zß水🍌🍌");
        }
    }

    SECTION("convert full dataset with nul characters") {
        src.resize(6);
        src[0] = u'\0';
        src[1] = u't';
        src[2] = u'e';
        src[3] = u's';
        src[4] = u't';
        src[5] = u'\0';
        srcCount = 6;
        dst.resize(128);
        std::fill(dst.begin(), dst.end(), UINT8_MAX);
        dstCount = dst.size();

        result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
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
            result = foe_utf16_to_utf8(&srcCount, nullptr, &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only src buffer") {
            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only dst buffer") {
            result = foe_utf16_to_utf8(&srcCount, nullptr, &dstCount, (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with both src & dst buffers") {
            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("zero-sized destination buffer") {
        SECTION("with zero-sized source succeeds") {
            src = u"";
            srcCount = 0;
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with non-zero source returns incomplete") {
            src = u"z";
            srcCount = src.size();
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("incomplete input multi-byte codepoint returns malformed") {
        SECTION("starting with an error means no data converted") {
            SECTION("missing first surrogate of '🍌'") {
                src = u"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data() + 1, &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing last surrogate of '🍌'") {
                src = u"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
        }
        SECTION("having an issue partially through a conversion returns what could be successfully "
                "converted") {
            SECTION("missing first surrogate of '🍌'") {
                src = u"test🍌";
                src[4] = src[5];
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
            }
            SECTION("missing last surrogate of '🍌'") {
                src = u"test🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
            }
            SECTION("repeating first surrogate of '🍌'") {
                src = u"🍌";
                src[1] = src[0];
                srcCount = src.size();
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing first surrogate of second '🍌'") {
                src = u"🍌🍌";
                src[2] = src[3];
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 2);
                CHECK(dstCount == 4);
                dst.resize(dstCount);
                CHECK(dst == u8"🍌");
            }
            SECTION("missing last surrogate of second '🍌'") {
                src = u"🍌🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                           (uint8_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 2);
                CHECK(dstCount == 4);
                dst.resize(dstCount);
                CHECK(dst == u8"🍌");
            }
        }
    }

    SECTION("with multi-byte codepoint insufficient destination room returns incomplete") {
        SECTION("starting with an error means no data converted") {
            src = u"🍌";
            srcCount = src.size();
            dst.resize(3);
            dstCount = 3;

            result = foe_utf16_to_utf8(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                       (uint8_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("check codepoint ranges") {
        dst.resize(128);

        SECTION("valid range 0x00 - 0x7F (1-byte)") {
            for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                uint16_t src = codepoint;
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf16_to_utf8(&srcCount, &src, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 1);
            }
        }
        SECTION("valid range 0x080 - 0x07FF (2-byte)") {
            for (uint32_t codepoint = 0x080; codepoint <= 0x07FF; ++codepoint) {
                uint16_t src = codepoint;
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf16_to_utf8(&srcCount, &src, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 2);
            }
        }
        SECTION("valid range 0x0800 - 0xD7FF (3-byte)") {
            for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                uint16_t src = codepoint;
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf16_to_utf8(&srcCount, &src, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 3);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF (3-byte)") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                uint16_t src = codepoint;
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf16_to_utf8(&srcCount, &src, &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 3);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF (4-byte)") {
            for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                uint32_t adjustedCodepoint = codepoint - 0x10000;
                std::array<uint16_t, 2> src;
                src[0] = 0xD800 | ((adjustedCodepoint >> 10) & 0x03FF);
                src[1] = 0xDC00 | (adjustedCodepoint & 0x03FF);

                srcCount = 2;
                dstCount = dst.size();

                result = foe_utf16_to_utf8(&srcCount, &src[0], &dstCount, (uint8_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 2);
                CHECK(dstCount == 4);
            }
        }
    }
}

TEST_CASE("UTF-16 to UTF-32 - string conversion") {
    std::u16string src;
    size_t srcCount;
    std::u32string dst;
    size_t dstCount = 0;
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("ハロー・ワールド") {
            src = u"ハロー・ワールド";
            srcCount = src.size();

            result =
                foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 8);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                        (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 8);
            CHECK(dstCount == 8);
            CHECK(dst == U"ハロー・ワールド");
        }

        SECTION("zß水🍌") {
            src = u"zß水🍌🍌";
            srcCount = src.size();

            result =
                foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 7);
            CHECK(dstCount == 5);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                        (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 7);
            CHECK(dstCount == 5);
            CHECK(dst == U"zß水🍌🍌");
        }
    }

    SECTION("convert full dataset with nul characters") {
        src.resize(6);
        src[0] = u'\0';
        src[1] = u't';
        src[2] = u'e';
        src[3] = u's';
        src[4] = u't';
        src[5] = u'\0';
        srcCount = 6;
        dst.resize(128);
        std::fill(dst.begin(), dst.end(), UINT8_MAX);
        dstCount = dst.size();

        result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                    (uint32_t *)dst.data());

        CHECK(result == FOE_SUCCESS);
        CHECK(srcCount == 6);
        CHECK(dstCount == 6);
        CHECK(memcmp(dst.data(), U"\0test\0", dstCount) == 0);
    }

    SECTION("zero-sized source leads to 'successful' zero-sized conversion") {
        srcCount = 0;
        dst.resize(128);
        dst[0] = UINT16_MAX;
        dstCount = 128;

        SECTION("with no buffers") {
            result = foe_utf16_to_utf32(&srcCount, nullptr, &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only src buffer") {
            result =
                foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only dst buffer") {
            result = foe_utf16_to_utf32(&srcCount, nullptr, &dstCount, (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with both src & dst buffers") {
            result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                        (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("zero-sized destination buffer") {
        SECTION("with zero-sized source succeeds") {
            src = u"";
            srcCount = 0;
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                        (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with non-zero source returns incomplete") {
            src = u"z";
            srcCount = src.size();
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                        (uint32_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("with multi-byte codepoint insufficient destination room returns incomplete") {
        SECTION("starting with an error means no data converted") {
            src = u"🍌🍌";
            srcCount = src.size();
            dst.resize(1);
            dstCount = 1;

            result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                        (uint32_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 2);
            CHECK(dstCount == 1);
            dst.resize(dstCount);
            CHECK(dst == U"🍌");
        }
    }

    SECTION("incomplete input multi-byte codepoint returns malformed") {
        SECTION("starting with an error means no data converted") {
            SECTION("missing first surrogate of '🍌'") {
                src = u"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data() + 1, &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing last surrogate of '🍌'") {
                src = u"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("repeating first surrogate of '🍌'") {
                src = u"🍌";
                src[1] = src[0];
                srcCount = src.size();
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
        }
        SECTION("having an issue partially through a conversion returns what could be successfully "
                "converted") {
            SECTION("missing first surrogate of '🍌'") {
                src = u"test🍌";
                src[4] = src[5];
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
            }
            SECTION("missing last surrogate of '🍌'") {
                src = u"test🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
            }
            SECTION("missing first surrogate of second '🍌'") {
                src = u"🍌🍌";
                src[2] = src[3];
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 2);
                CHECK(dstCount == 1);
                dst.resize(dstCount);
                CHECK(dst == U"🍌");
            }
            SECTION("missing last surrogate of second '🍌'") {
                src = u"🍌🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf16_to_utf32(&srcCount, (uint16_t const *)src.data(), &dstCount,
                                            (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 2);
                CHECK(dstCount == 1);
                dst.resize(dstCount);
                CHECK(dst == U"🍌");
            }
        }
    }

    SECTION("check codepoint ranges") {
        dst.resize(128);

        SECTION("valid range 0x0000 - 0xD7FF") {
            for (uint32_t codepoint = 0x0800; codepoint < 0xD7FF; ++codepoint) {
                uint16_t src = codepoint;
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf16_to_utf32(&srcCount, &src, &dstCount, (uint32_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 1);
            }
        }
        SECTION("valid range 0xE000 - 0xFFFF") {
            for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                uint16_t src = codepoint;
                srcCount = 1;
                dstCount = dst.size();

                result = foe_utf16_to_utf32(&srcCount, &src, &dstCount, (uint32_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 1);
                CHECK(dstCount == 1);
            }
        }
        SECTION("valid range 0x010000 - 0x10FFFF") {
            for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                uint32_t adjustedCodepoint = codepoint - 0x10000;
                std::array<uint16_t, 2> src;
                src[0] = 0xD800 | ((adjustedCodepoint >> 10) & 0x03FF);
                src[1] = 0xDC00 | (adjustedCodepoint & 0x03FF);

                srcCount = 2;
                dstCount = dst.size();

                result = foe_utf16_to_utf32(&srcCount, &src[0], &dstCount, (uint32_t *)dst.data());

                CHECK(result == FOE_SUCCESS);
                CHECK(srcCount == 2);
                CHECK(dstCount == 1);
            }
        }
    }
}