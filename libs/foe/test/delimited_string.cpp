/*
    Copyright (C) 2022 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

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
        CHECK(foeCreateDelimitedString(0, nullptr, &dstLen, nullptr));

        CHECK(dstLen == 0);
    }
    SECTION("With destination buffer") {
        dstArr[0] = 'a';
        CHECK(foeCreateDelimitedString(0, nullptr, &dstLen, dstArr));

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
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 0);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }

    SECTION("String smaller than buffer") {
        std::vector<char const *> srcStrings{"abcd"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 5);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 5);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
        }
    }

    SECTION("String smaller than buffer by 1 (perfect fit)") {
        std::vector<char const *> srcStrings{"abcdefg"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 8);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
        }
    }

    SECTION("String same size of buffer (too big by 1)") {
        std::vector<char const *> srcStrings{"abcdefgh"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 9);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK_FALSE(
                foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }

    SECTION("String much bigger than buffer ") {
        std::vector<char const *> srcStrings{"abcdefghijklmnopqrstuvwxyz"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 27);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK_FALSE(
                foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

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
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 0);
        }
        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 0);
            CHECK(dstArr[0] == 'a');
        }
    }

    SECTION("Some zero-length strings") {
        std::vector<char const *> srcStrings{"", "a", "", "c", ""};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 4);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 4);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[1]});
            CHECK(std::string_view{dstArr + 2} == std::string_view{srcStrings[3]});
        }
    }

    SECTION("Strings that, in total, match the buffer size") {
        std::vector<char const *> srcStrings{"abc", "def"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 8);
        }
        SECTION("Destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == std::string_view{srcStrings[0]});
            CHECK(std::string_view{dstArr + 4} == std::string_view{srcStrings[1]});
        }
    }

    SECTION("Strings that, in total, exceed the buffer size") {
        std::vector<char const *> srcStrings{"abcd", "efgh"};

        SECTION("No destination buffer") {
            CHECK(foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, nullptr));

            CHECK(dstLen == 10);
        }
        SECTION("Destination buffer") {
            CHECK_FALSE(
                foeCreateDelimitedString(srcStrings.size(), srcStrings.data(), &dstLen, dstArr));

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
        CHECK(foeCopyDelimitedString(srcString.size(), srcString.data(), &dstLen, nullptr));

        CHECK(dstLen == 0);
    }

    SECTION("Destination buffer") {
        dstArr[0] = 'a';
        CHECK(foeCopyDelimitedString(srcString.size(), srcString.data(), &dstLen, nullptr));

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
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), &dstLen, nullptr));

            CHECK(dstLen == 5);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), &dstLen, dstArr));

            CHECK(dstLen == 5);
            CHECK(std::string_view{dstArr} == srcString);
        }
    }

    SECTION("Same size as than destination buffer") {
        std::string srcString = "abcdefg";
        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), &dstLen, nullptr));

            CHECK(dstLen == 8);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), &dstLen, dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == srcString);
        }
    }

    SECTION("Larger than destination buffer") {
        std::string srcString = "abcdefgh";
        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcString.size() + 1, srcString.data(), &dstLen, nullptr));

            CHECK(dstLen == 9);
        }

        SECTION("Destination buffer") {
            dstArr[0] = 'a';
            CHECK_FALSE(
                foeCopyDelimitedString(srcString.size() + 1, srcString.data(), &dstLen, dstArr));

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
            CHECK(foeCopyDelimitedString(srcLen, srcString, &dstLen, nullptr));

            CHECK(dstLen == 6);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, &dstLen, dstArr));

            CHECK(dstLen == 6);
            CHECK(memcmp(srcString, dstArr, dstLen) == 0);
        }
    }

    SECTION("Same size as than destination buffer") {
        char const *srcString = "abc\0efg";
        size_t srcLen = 8;

        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, &dstLen, nullptr));

            CHECK(dstLen == 8);
        }

        SECTION("Destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, &dstLen, dstArr));

            CHECK(dstLen == 8);
            CHECK(std::string_view{dstArr} == srcString);
        }
    }

    SECTION("Larger than destination buffer") {
        char const *srcString = "abcd\0efgh";
        size_t srcLen = 10;

        SECTION("No destination buffer") {
            CHECK(foeCopyDelimitedString(srcLen, srcString, &dstLen, nullptr));

            CHECK(dstLen == 10);
        }

        SECTION("Destination buffer") {
            CHECK_FALSE(foeCopyDelimitedString(srcLen, srcString, &dstLen, dstArr));

            CHECK(dstLen == 5);
            CHECK(memcmp(srcString, dstArr, dstLen) == 0);
        }
    }
}

TEST_CASE("CombinedString - Getting an indexed string from a zero-length combined string",
          "[foe][CombinedString]") {
    char const *srcString = "";
    size_t srcLen = 0;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 0, &pStr));

        CHECK(pStr == nullptr);
    }
    SECTION("Index 1") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 1, &pStr));

        CHECK(pStr == nullptr);
    }
}

TEST_CASE("CombinedString - Getting an indexed string from a single combined string",
          "[foe][CombinedString]") {
    char const *srcString = "abcd";
    size_t srcLen = 4;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, &pStr));

        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr} == std::string_view{pStr});
    }
    SECTION("Index 1") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 1, &pStr));

        CHECK(pStr == nullptr);
    }
}

TEST_CASE("CombinedString - Getting an indexed string from a combined multi-string",
          "[foe][CombinedString]") {
    char const *srcString = "abcd\0efgh";
    size_t srcLen = 9;
    char const *pStr = nullptr;

    SECTION("Index 0") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 0, &pStr));

        CHECK(pStr == srcString);
        CHECK(std::string_view{pStr} == std::string_view{"abcd"});
    }
    SECTION("Index 1") {
        CHECK(foeIndexedDelimitedString(srcLen, srcString, 1, &pStr));

        CHECK(pStr == srcString + 5);
        CHECK(std::string_view{pStr} == std::string_view{"efgh"});
    }
    SECTION("Index 2") {
        CHECK_FALSE(foeIndexedDelimitedString(srcLen, srcString, 2, &pStr));

        CHECK(pStr == nullptr);
    }
}
