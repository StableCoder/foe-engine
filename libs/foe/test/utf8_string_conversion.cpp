// Copyright (C) 2026 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/utf_string_conversion.h>

#include <array>
#include <cstdint>
#include <string>

TEST_CASE("UTF-8 to UTF-16 - string conversion") {
    std::u8string src;
    size_t srcCount = 0;
    std::u16string dst;
    size_t dstCount = 0;
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("ハロー・ワールド") {
            src = u8"ハロー・ワールド";
            srcCount = src.size();

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 24);
            CHECK(dstCount == 8);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 24);
            CHECK(dstCount == 8);
            CHECK(dst == u"ハロー・ワールド");
        }

        SECTION("zß水🍌🍌") {
            src = u8"zß水🍌🍌";
            srcCount = src.size();

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 14);
            CHECK(dstCount == 7);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 14);
            CHECK(dstCount == 7);
            CHECK(dst == u"zß水🍌🍌");
        }
    }

    SECTION("convert full dataset with nul characters") {
        src.resize(6);
        src[0] = u8'\0';
        src[1] = u8't';
        src[2] = u8'e';
        src[3] = u8's';
        src[4] = u8't';
        src[5] = u8'\0';
        srcCount = 6;
        dst.resize(128);
        std::fill(dst.begin(), dst.end(), UINT16_MAX);
        dstCount = dst.size();

        result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
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
            result = foe_utf8_to_utf16(&srcCount, nullptr, &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only src buffer") {
            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only dst buffer") {
            result = foe_utf8_to_utf16(&srcCount, nullptr, &dstCount, (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with both src & dst buffers") {
            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("zero-sized destination buffer") {
        SECTION("with zero-sized source succeeds") {
            src = u8"";
            srcCount = 0;
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with non-zero source returns incomplete") {
            src = u8"z";
            srcCount = src.size();
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("malformed multi-byte input failures") {
        SECTION("starting with an error means no data converted") {
            SECTION("missing first byte of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data() + 1, &dstCount,
                                           (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing last byte of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
        }
        SECTION("having an issue partially through a conversion returns what could be successfully "
                "converted") {
            SECTION("missing first byte of '🍌'") {
                src = u8"test🍌";
                std::move(src.begin() + 5, src.end(), src.begin() + 4);
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
                dst.resize(dstCount);
                CHECK(dst == u"test");
            }
            SECTION("missing last byte of '🍌'") {
                src = u8"test🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
                dst.resize(dstCount);
                CHECK(dst == u"test");
            }
            SECTION("missing first byte of second '🍌'") {
                src = u8"🍌🍌";
                std::move(src.begin() + 5, src.end(), src.begin() + 4);
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 4);
                CHECK(dstCount == 2);
                dst.resize(dstCount);
                CHECK(dst == u"🍌");
            }
            SECTION("missing last byte of second '🍌'") {
                src = u8"🍌🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint16_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 4);
                CHECK(dstCount == 2);
                dst.resize(dstCount);
                CHECK(dst == u"🍌");
            }
        }
    }

    SECTION("with multi-byte codepoint insufficient destination room returns incomplete") {
        SECTION("starting with an error means no data converted") {
            src = u8"🍌";
            srcCount = src.size();
            dst.resize(1);
            dstCount = 1;

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("having an issue partially through a conversion returns what could be successfully "
                "converted") {
            src = u8"test🍌";
            srcCount = src.size();
            dst.resize(5);
            dstCount = 5;

            result = foe_utf8_to_utf16(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint16_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 4);
            CHECK(dstCount == 4);
            dst.resize(dstCount);
            CHECK(dst == u"test");
        }
    }

    SECTION("check codepoint ranges") {
        SECTION("invalid single-byte of 0x80") {
            uint8_t src = 0x80;
            srcCount = 1;
            dst.resize(128);
            dstCount = 128;

            result = foe_utf8_to_utf16(&srcCount, &src, &dstCount, (uint16_t *)dst.data());

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("invalid single-byte of 0xFF") {
            uint8_t src = 0xFF;
            srcCount = 1;
            dst.resize(128);
            dstCount = 128;

            result = foe_utf8_to_utf16(&srcCount, &src, &dstCount, (uint16_t *)dst.data());

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("codepoint ranges") {
            SECTION("1-byte") {
                SECTION("valid range 0x00 - 0x7F") {
                    uint8_t src;
                    dst.resize(128);

                    for (uint32_t codepoint = 0; codepoint <= 0x7F; ++codepoint) {
                        src = codepoint;

                        srcCount = 1;
                        dstCount = dst.size();

                        result =
                            foe_utf8_to_utf16(&srcCount, &src, &dstCount, (uint16_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 1);
                        CHECK(dstCount == 1);
                    }
                }
            }

            SECTION("2-byte") {
                std::array<uint8_t, 2> src;
                dst.resize(128);

                SECTION("invalid range 0x00 - 0x7F") {
                    for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0x0080 - 0x07FF") {
                    for (uint32_t codepoint = 0x0080; codepoint <= 0x07FF; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 2);
                        CHECK(dstCount == 1);
                    }
                }
            }

            SECTION("3-byte") {
                std::array<uint8_t, 3> src;
                dst.resize(128);

                SECTION("invalid range 0x0000 - 0x0800") {
                    for (uint32_t codepoint = 0; codepoint < 0x0800; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0x0800 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 3);
                        CHECK(dstCount == 1);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 3);
                        CHECK(dstCount == 1);
                    }
                }
            }

            SECTION("4-byte") {
                std::array<uint8_t, 4> src;
                dst.resize(128);

                SECTION("invalid range 0x0000 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0000; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("invalid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0x010000 - 0x10FFFF") {
                    for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf16(&srcCount, src.data(), &dstCount,
                                                   (uint16_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 4);
                        CHECK(dstCount == 2);
                    }
                }
                SECTION("invalid range 0x110000+") {
                    uint32_t codepoint = 0x110000;
                    src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                    src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                    src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                    src[3] = 0x80 | (codepoint & 0x3F);

                    srcCount = src.size();
                    dstCount = dst.size();

                    result =
                        foe_utf8_to_utf16(&srcCount, src.data(), &dstCount, (uint16_t *)dst.data());

                    CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                    CHECK(srcCount == 0);
                    CHECK(dstCount == 0);
                }
            }
        }
    }
}

TEST_CASE("UTF-8 to UTF-32 - string conversion") {
    std::u8string src;
    size_t srcCount;
    std::u32string dst;
    size_t dstCount = 0;
    foeResult result;

    SECTION("successful regular conversions") {
        SECTION("ハロー・ワールド") {
            src = u8"ハロー・ワールド";
            srcCount = src.size();

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 24);
            CHECK(dstCount == 8);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 24);
            CHECK(dstCount == 8);
            CHECK(dst == U"ハロー・ワールド");
        }

        SECTION("zß水c") {
            src = u8"zß水🍌🍌";
            srcCount = src.size();

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 14);
            CHECK(dstCount == 5);

            dst.resize(dstCount);
            srcCount = src.size();

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 14);
            CHECK(dstCount == 5);
            CHECK(dst == U"zß水🍌🍌");
        }
    }

    SECTION("convert full dataset with nul characters") {
        src.resize(6);
        src[0] = u8'\0';
        src[1] = u8't';
        src[2] = u8'e';
        src[3] = u8's';
        src[4] = u8't';
        src[5] = u8'\0';
        srcCount = 6;
        dst.resize(128);
        std::fill(dst.begin(), dst.end(), UINT32_MAX);
        dstCount = dst.size();

        result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                   (uint32_t *)dst.data());

        CHECK(result == FOE_SUCCESS);
        CHECK(srcCount == 6);
        CHECK(dstCount == 6);
        CHECK(memcmp(dst.data(), U"\0test\0", dstCount) == 0);
    }

    SECTION("zero-sized source leads to 'successful' zero-sized conversion") {
        srcCount = 0;
        dst.resize(128);
        dst[0] = UINT32_MAX;
        dstCount = 128;

        SECTION("with no buffers") {
            result = foe_utf8_to_utf32(&srcCount, nullptr, &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only src buffer") {
            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount, nullptr);

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with only dst buffer") {
            result = foe_utf8_to_utf32(&srcCount, nullptr, &dstCount, (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with both src & dst buffers") {
            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("zero-sized destination buffer") {
        SECTION("with zero-sized source succeeds") {
            src = u8"";
            srcCount = 0;
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint32_t *)dst.data());

            CHECK(result == FOE_SUCCESS);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("with non-zero source returns incomplete") {
            src = u8"z";
            srcCount = src.size();
            dst.resize(src.size());
            dstCount = 0;

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint32_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
    }

    SECTION("malformed multi-byte input failures") {
        SECTION("starting with an error means no data converted") {
            SECTION("starting with second byte of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data() + 1, &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("starting with third byte of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 2;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data() + 2, &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("starting with fourth byte of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 3;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data() + 3, &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing last 3 bytes of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 3;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing last 2 bytes of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 2;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("missing last 1 bytes of '🍌'") {
                src = u8"🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("first byte of '🍌' and then 'z'") {
                src = u8"🍌";
                src[1] = 'z';
                srcCount = 2;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("second byte of '🍌' and then 'z'") {
                src = u8"🍌z";
                src[0] = src[1];
                src[1] = src[4];
                srcCount = 2;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("third byte of '🍌' and then 'z'") {
                src = u8"🍌z";
                src[0] = src[2];
                src[1] = src[4];
                srcCount = 2;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
            SECTION("fourth byte of '🍌' and then 'z'") {
                src = u8"🍌z";
                src[0] = src[3];
                src[1] = src[4];
                srcCount = 2;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 0);
                CHECK(dstCount == 0);
            }
        }
        SECTION("having an issue partially through a conversion returns what could be successfully "
                "converted") {
            SECTION("missing first byte of '🍌'") {
                src = u8"test🍌";
                std::move(src.begin() + 5, src.end(), src.begin() + 4);
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
            }
            SECTION("missing last byte of '🍌'") {
                src = u8"test🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 4);
                CHECK(dstCount == 4);
            }
            SECTION("missing first byte of second '🍌'") {
                src = u8"🍌🍌";
                std::move(src.begin() + 5, src.end(), src.begin() + 4);
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                CHECK(srcCount == 4);
                CHECK(dstCount == 1);
                dst.resize(dstCount);
                CHECK(dst == U"🍌");
            }
            SECTION("missing last byte of second '🍌'") {
                src = u8"🍌🍌";
                srcCount = src.size() - 1;
                dst.resize(128);
                dstCount = 128;

                result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                           (uint32_t *)dst.data());

                CHECK(result == FOE_ERROR_UTF_MB_INCOMPLETE);
                CHECK(srcCount == 4);
                CHECK(dstCount == 1);
                dst.resize(dstCount);
                CHECK(dst == U"🍌");
            }
        }
    }

    SECTION("with multi-byte codepoint insufficient destination room returns incomplete") {
        SECTION("starting with an error means no data converted") {
            src = u8"🍌🍌";
            srcCount = src.size();
            dst.resize(1);
            dstCount = 1;

            result = foe_utf8_to_utf32(&srcCount, (uint8_t const *)src.data(), &dstCount,
                                       (uint32_t *)dst.data());

            CHECK(result == FOE_INCOMPLETE);
            CHECK(srcCount == 4);
            CHECK(dstCount == 1);
            dst.resize(dstCount);
            CHECK(dst == U"🍌");
        }
    }

    SECTION("check codepoint ranges") {
        SECTION("invalid single-byte of 0x80") {
            uint8_t src = 0x80;
            srcCount = 1;
            dst.resize(128);
            dstCount = 128;

            result = foe_utf8_to_utf32(&srcCount, &src, &dstCount, (uint32_t *)dst.data());

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("invalid single-byte of 0xFF") {
            uint8_t src = 0xFF;
            srcCount = 1;
            dst.resize(128);
            dstCount = 128;

            result = foe_utf8_to_utf32(&srcCount, &src, &dstCount, (uint32_t *)dst.data());

            CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
            CHECK(srcCount == 0);
            CHECK(dstCount == 0);
        }
        SECTION("codepoint ranges") {
            SECTION("1-byte") {
                SECTION("valid range 0x00 - 0x7F") {
                    uint8_t src;
                    dst.resize(128);

                    for (uint32_t codepoint = 0; codepoint <= 0x7F; ++codepoint) {
                        src = codepoint;

                        srcCount = 1;
                        dstCount = dst.size();

                        result =
                            foe_utf8_to_utf32(&srcCount, &src, &dstCount, (uint32_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 1);
                        CHECK(dstCount == 1);
                    }
                }
            }
            SECTION("2-byte") {
                std::array<uint8_t, 2> src;
                dst.resize(128);

                SECTION("invalid range 0x00 - 0x7F") {
                    for (uint32_t codepoint = 0x00; codepoint <= 0x7F; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0x0080 - 0x07FF") {
                    for (uint32_t codepoint = 0x0080; codepoint <= 0x07FF; ++codepoint) {
                        src[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
                        src[1] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 2);
                        CHECK(dstCount == 1);
                    }
                }
            }
            SECTION("3-byte") {
                std::array<uint8_t, 3> src;
                dst.resize(128);

                SECTION("invalid range 0x0000 - 0x0800") {
                    for (uint32_t codepoint = 0; codepoint < 0x0800; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0x0800 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0800; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 3);
                        CHECK(dstCount == 1);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
                        src[1] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[2] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 3);
                        CHECK(dstCount == 1);
                    }
                }
            }
            SECTION("4-byte") {
                std::array<uint8_t, 4> src;
                dst.resize(128);

                SECTION("invalid range 0x0000 - 0xD7FF") {
                    for (uint32_t codepoint = 0x0000; codepoint <= 0xD7FF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("invalid range 0xD800 - 0xDFFF") {
                    for (uint32_t codepoint = 0xD800; codepoint <= 0xDFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("invalid range 0xE000 - 0xFFFF") {
                    for (uint32_t codepoint = 0xE000; codepoint <= 0xFFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_ERROR_UTF_MALFORMED_DATA);
                        CHECK(srcCount == 0);
                        CHECK(dstCount == 0);
                    }
                }
                SECTION("valid range 0x010000 - 0x10FFFF") {
                    for (uint32_t codepoint = 0x010000; codepoint <= 0x10FFFF; ++codepoint) {
                        src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                        src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                        src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                        src[3] = 0x80 | (codepoint & 0x3F);

                        srcCount = src.size();
                        dstCount = dst.size();

                        result = foe_utf8_to_utf32(&srcCount, src.data(), &dstCount,
                                                   (uint32_t *)dst.data());

                        CHECK(result == FOE_SUCCESS);
                        CHECK(srcCount == 4);
                        CHECK(dstCount == 1);
                    }
                }
                SECTION("invalid range 0x110000+") {
                    uint32_t codepoint = 0x110000;
                    src[0] = 0xF0 | ((codepoint >> 18) & 0x07);
                    src[1] = 0x80 | ((codepoint >> 12) & 0x3F);
                    src[2] = 0x80 | ((codepoint >> 6) & 0x3F);
                    src[3] = 0x80 | (codepoint & 0x3F);

                    srcCount = src.size();
                    dstCount = dst.size();

                    result =
                        foe_utf8_to_utf32(&srcCount, src.data(), &dstCount, (uint32_t *)dst.data());

                    CHECK(result == FOE_ERROR_UTF_INVALID_CODEPOINT);
                    CHECK(srcCount == 0);
                    CHECK(dstCount == 0);
                }
            }
        }
    }
}
