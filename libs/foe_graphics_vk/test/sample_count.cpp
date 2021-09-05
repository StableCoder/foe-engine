/*
    Copyright (C) 2021 George Cave.

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
#include <foe/graphics/vk/sample_count.hpp>

TEST_CASE("SampleCount - Flag to integer", "[foe][graphics][vk]") {
    SECTION("Valid flags return correct counts") {
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_1_BIT) == 1);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_2_BIT) == 2);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_4_BIT) == 4);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_8_BIT) == 8);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_16_BIT) == 16);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_32_BIT) == 32);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_64_BIT) == 64);
    }

    SECTION("Invalid flags return 0") {
        REQUIRE(foeGfxVkGetSampleCount(0) == 0);
        REQUIRE(foeGfxVkGetSampleCount(0x11) == 0);
        REQUIRE(foeGfxVkGetSampleCount(0x111) == 0);
        REQUIRE(foeGfxVkGetSampleCount(VK_SAMPLE_COUNT_64_BIT + 1) == 0);
    }
}

TEST_CASE("SampleCount - Integer to flag", "[foe][graphics][vk]") {
    SECTION("Invalid integers return a non-flag (zero)") {
        REQUIRE(foeGfxVkGetSampleCountFlags(0) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(3) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(5) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(6) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(7) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(9) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(15) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(17) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(31) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(33) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(63) == 0);
        REQUIRE(foeGfxVkGetSampleCountFlags(65) == 0);
    }

    SECTION("Valid integers return a real flag") {
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_1_BIT) == 1);
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_2_BIT) == 2);
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_4_BIT) == 4);
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_8_BIT) == 8);
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_16_BIT) == 16);
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_32_BIT) == 32);
        REQUIRE(foeGfxVkGetSampleCountFlags(VK_SAMPLE_COUNT_64_BIT) == 64);
    }
}