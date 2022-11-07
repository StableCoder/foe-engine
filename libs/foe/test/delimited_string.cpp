// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/delimited_string.h>

#include <cstring>
#include <string>

TEST_CASE("CombinedString - Combining no string", "[foe][CombinedString]") {
    char dstArr[8];
    uint32_t dstLen = sizeof(dstArr);
    memset(dstArr, 0, sizeof(dstArr));

    REQUIRE(dstArr[0] == '\0');

    SECTION("No destination buffer") {
        CHECK(foeCreateDelimitedString(0, nullptr, '\0', &dstLen, nullptr));

        CHECK(dstLen == 0);
    }
    SECTION("With destination buffer") {
        dstArr[0] = 'a';
        CHECK(foeCreateDelimitedString(0, nullptr, '\0', &dstLen, dstArr));

        CHECK(dstLen == 0);
        CHECK(dstArr[0] == 'a');
    }
}

TEST_CASE("CombinedString - Combining a single given string", "[foe][CombinedString]") {
    char dstArr[8];
    uint32_t dstLen = sizeof(dstArr);
    memset(dstArr, 0, sizeof(dstArr));

    SECTION("Zero-length string") {
        std::vector<char const *> srcStrings{""};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 0);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }

    SECTION("String smaller than buffer") {
        std::vector<char const *> srcStrings{"abcd"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 5);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           dstArr));

            CHECK(dstLen == 5);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
        }
    }

    SECTION("String smaller than buffer by 1 (perfect fit)") {
        std::vector<char const *> srcStrings{"abcdefg"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 8);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
        }
    }

    SECTION("String same size of buffer (too big by 1)") {
        std::vector<char const *> srcStrings{"abcdefgh"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 9);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK_FALSE(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0',
                                                 &dstLen, dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }

    SECTION("String much bigger than buffer ") {
        std::vector<char const *> srcStrings{"abcdefghijklmnopqrstuvwxyz"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 27);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK_FALSE(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0',
                                                 &dstLen, dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }
}

TEST_CASE("CombinedString - Combining a multiple given strings", "[foe][CombinedString]") {
    char dstArr[8];
    uint32_t dstLen = sizeof(dstArr);
    memset(dstArr, 0, sizeof(dstArr));

    SECTION("Zero-length strings") {
        std::vector<char const *> srcStrings{"", ""};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 0);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }

    SECTION("Some zero-length strings") {
        std::vector<char const *> srcStrings{"", "a", "", "c", ""};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 4);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           dstArr));

            CHECK(dstLen == 4);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[1]});
            CHECK(std::string_view{dstArr + 2} == std::string_view{srcStrings[3]});
        }
    }

    SECTION("Strings that, in total, match the buffer size") {
        std::vector<char const *> srcStrings{"abc", "def"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 8);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
            CHECK(std::string_view{dstArr + 4} == std::string_view{srcStrings[1]});
        }
    }

    SECTION("Strings that, in total, exceed the buffer size") {
        std::vector<char const *> srcStrings{"abcd", "efgh"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0', &dstLen,
                                           nullptr));

            CHECK(dstLen == 10);
        }
        SECTION("Destination buffer") {
            CHECK_FALSE(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), '\0',
                                                 &dstLen, dstArr));

            CHECK(dstLen == 5);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
        }
    }
}

TEST_CASE("CombinedString - Copying a zero-length combined string", "[foe][CombinedString]") {
    std::string srcString = "";
    char dstArr[8];
    uint32_t dstLen = sizeof(dstArr);
    memset(dstArr, 0, sizeof(dstArr));

    SECTION("No destination buffer") {
        CHECK(foeCopyDelimitedString(srcString.size(), srcString.data(), '\0', &dstLen, nullptr));

        CHECK(dstLen == 0);
    }

    SECTION("Destination buffer") {
        dstArr[0] = 'a';
        CHECK(foeCopyDelimitedString(srcString.size(), srcString.data(), '\0', &dstLen, nullptr));

        CHECK(dstLen == 0);
        CHECK(dstArr[0] == 'a');
    }
}

TEST_CASE("CombinedString - Copying a single combined string", "[foe][CombinedString]") {
    char dstArr[8];
    uint32_t dstLen = sizeof(dstArr);
    memset(dstArr, 0, sizeof(dstArr));

    SECTION("Smaller than destination buffer") {
        std::string srcString = "abcd";
        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), '\0', &dstLen,
                                         nullptr));

            CHECK(dstLen == 5);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), '\0', &dstLen,
                                         dstArr));

            CHECK(dstLen == 5);
            CHECK(std::string_view{dstArr} == srcString);
        }
    }

    SECTION("Same size as than destination buffer") {
        std::string srcString = "abcdefg";
        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), '\0', &dstLen,
                                         nullptr));

            CHECK(dstLen == 8);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), '\0', &dstLen,
                                         dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == srcString);
        }
    }

    SECTION("Larger than destination buffer") {
        std::string srcString = "abcdefgh";
        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), '\0', &dstLen,
                                         nullptr));

            CHECK(dstLen == 9);
        }

        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK_FALSE(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), '\0',
                                               &dstLen, dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }
}

TEST_CASE("CombinedString - Copying a multi-combined string", "[foe][CombinedString]") {
    char dstArr[8];
    uint32_t dstLen = sizeof(dstArr);
    memset(dstArr, 0, sizeof(dstArr));

    SECTION("Smaller than destination buffer") {
        char const *srcString = "ab\0de";
        size_t srcLen = 6;

        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, '\0', &dstLen, nullptr));

            CHECK(dstLen == 6);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, '\0', &dstLen, dstArr));

            CHECK(dstLen == 6);
            CHECK(memcmp(srcString, dstArr, dstLen) == 0);
        }
    }

    SECTION("Same size as than destination buffer") {
        char const *srcString = "abc\0efg";
        size_t srcLen = 8;

        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, '\0', &dstLen, nullptr));

            CHECK(dstLen == 8);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, '\0', &dstLen, dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == srcString);
        }
    }

    SECTION("Larger than destination buffer") {
        char const *srcString = "abcd\0efgh";
        size_t srcLen = 10;

        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, '\0', &dstLen, nullptr));

            CHECK(dstLen == 10);
        }

        SECTION("Destination buffer") {
            CHECK_FALSE(foeCopyDelimitedString(srcLen, srcString, '\0', &dstLen, dstArr));

            CHECK(dstLen == 5);
            CHECK(memcmp(srcString, dstArr, dstLen) == 0);
        }
    }
}

TEST_CASE("CombinedString - Getting an indexed string from a zero-length combined string",
          "[foe][CombinedString]") {
    char const *srcString = "";
    size_t srcLen = 0;

    uint32_t strLen = UINT32_MAX;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 0, '\0', &strLen, &pStr));

        CHECK(strLen == UINT32_MAX);
        CHECK(pStr == nullptr);
    }
    SECTION("Index 1") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 1, '\0', &strLen, &pStr));

        CHECK(strLen == UINT32_MAX);
        CHECK(pStr == nullptr);
    }
}

TEST_CASE("CombinedString - Getting an indexed string from a single combined string",
          "[foe][CombinedString]") {
    char const *srcString = "abcd";
    size_t srcLen = 4;

    uint32_t strLength = UINT32_MAX;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, '\0', nullptr, &pStr));

        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr} == std::string_view{pStr});

        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, '\0', &strLength, &pStr));

        CHECK(strLength == srcLen);
        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr} == std::string_view{pStr});
    }
    SECTION("Index 1") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 1, '\0', nullptr, &pStr));

        CHECK(pStr == nullptr);

        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 1, '\0', &strLength, &pStr));

        CHECK(strLength == UINT32_MAX);
        CHECK(pStr == nullptr);
    }
}

TEST_CASE("CombinedString - Getting an indexed string from a combined multi-string",
          "[foe][CombinedString]") {
    char const *srcString = "abcd\0efgh";
    size_t srcLen = 9;
    uint32_t strLength = UINT32_MAX;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, '\0', nullptr, &pStr));

        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr} == std::string_view{"abcd"});

        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, '\0', &strLength, &pStr));

        CHECK(strLength == 4);
        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr} == std::string_view{"abcd"});
    }
    SECTION("Index 1") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 1, '\0', nullptr, &pStr));

        CHECK(pStr == srcString + 5);
        CHECK(std::string_view{pStr} == std::string_view{"efgh"});

        CHECK(foeIndexedDelimitedString(srcLen, srcString, 1, '\0', &strLength, &pStr));

        CHECK(strLength == 4);
        CHECK(pStr == srcString + 5);
        CHECK(std::string_view{pStr} == std::string_view{"efgh"});
    }
    SECTION("Index 2") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 2, '\0', nullptr, &pStr));

        CHECK(pStr == nullptr);

        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 2, '\0', &strLength, &pStr));

        CHECK(strLength == UINT32_MAX);
        CHECK(pStr == nullptr);
    }
}

TEST_CASE(
    "CombinedString - Getting an indexed string from a combined multi-string (space delimited)",
    "[foe][CombinedString]") {
    char const *srcString = "abcd efgh";
    size_t srcLen = 9;
    uint32_t strLength = UINT32_MAX;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, ' ', &strLength, &pStr));

        CHECK(strLength == 4);
        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr, strLength} == std::string_view{"abcd"});
    }
    SECTION("Index 1") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 1, ' ', &strLength, &pStr));

        CHECK(strLength == 4);
        CHECK(pStr == srcString + 5);
        CHECK(std::string_view{pStr, strLength} == std::string_view{"efgh"});
    }
    SECTION("Index 2") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 2, ' ', nullptr, &pStr));

        CHECK(pStr == nullptr);

        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 2, ' ', &strLength, &pStr));

        CHECK(strLength == UINT32_MAX);
        CHECK(pStr == nullptr);
    }
}