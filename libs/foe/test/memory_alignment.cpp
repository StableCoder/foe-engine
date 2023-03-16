// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/memory_alignment.h>

TEST_CASE("foeGetAlignedSize") {
    SECTION("Data size of zero returns zero") {
        CHECK(foeGetAlignedSize(8, 0) == 0);
        CHECK(foeGetAlignedSize(64, 0) == 0);
        CHECK(foeGetAlignedSize(128, 0) == 0);
    }
    SECTION("Data size smaller than alignment") {
        CHECK(foeGetAlignedSize(64, 32) == 64);
        CHECK(foeGetAlignedSize(64, 63) == 64);
    }
    SECTION("Data size the same as alignment") {
        CHECK(foeGetAlignedSize(8, 8) == 8);
        CHECK(foeGetAlignedSize(64, 64) == 64);
    }
    SECTION("Data size larger than alignment") {
        CHECK(foeGetAlignedSize(8, 9) == 16);
        CHECK(foeGetAlignedSize(8, 15) == 16);
        CHECK(foeGetAlignedSize(8, 16) == 16);

        CHECK(foeGetAlignedSize(8, 8192) == 8192);

        CHECK(foeGetAlignedSize(64, 65) == 128);
        CHECK(foeGetAlignedSize(64, 127) == 128);
        CHECK(foeGetAlignedSize(64, 128) == 128);
    }
}