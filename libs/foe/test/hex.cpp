// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/hex.h>
#include <foe/result.h>

#include <array>
#include <string>

namespace {

std::string cUpperHexData = "A94A3C0F";
std::string cLowerHexData = "a94a3c0f";
constexpr uint32_t cBinaryData = 0x0F3C4AA9;

} // namespace

TEST_CASE("Binary/Hex Encode Cases") {
    std::array<char, (sizeof(cBinaryData) * 3)> hexData = {};
    foeResultSet result;

    SECTION("Success - Hex buffer size is exact size needed") {
        result = foeEncodeHex(sizeof(cBinaryData), &cBinaryData, (sizeof(cBinaryData) * 2) + 1,
                              hexData.data());
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(std::string(hexData.data()) == cUpperHexData);
    }

    SECTION("Success - Hex buffer size larger than size needed") {
        result = foeEncodeHex(sizeof(cBinaryData), &cBinaryData, sizeof(cBinaryData) * 3,
                              hexData.data());
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(std::string(hexData.data()) == cUpperHexData);
    }

    SECTION("Failure - Hex buffer size not large enough") {
        result = foeEncodeHex(sizeof(cBinaryData), &cBinaryData, sizeof(cBinaryData) * 2,
                              hexData.data());
        REQUIRE(result.value == FOE_ERROR_DESTINATION_BUFFER_TOO_SMALL);
    }
}

TEST_CASE("Binary/Hex Decode") {
    uint64_t binaryBuffer = 0;
    size_t binaryBufferSize = sizeof(binaryBuffer);
    foeResultSet result;

    SECTION("Success - Binary buffer is exact size needed") {
        binaryBufferSize = sizeof(cBinaryData);
        SECTION("Uppercase hex string") {
            result = foeDecodeHex(cUpperHexData.size(), cUpperHexData.data(), &binaryBufferSize,
                                  &binaryBuffer);
            CHECK(result.value == FOE_SUCCESS);

            CHECK(binaryBuffer == cBinaryData);
            CHECK(binaryBufferSize == sizeof(cBinaryData));
        }
        SECTION("Lowercase hex string") {
            result = foeDecodeHex(cUpperHexData.size(), cUpperHexData.data(), &binaryBufferSize,
                                  &binaryBuffer);
            CHECK(result.value == FOE_SUCCESS);

            CHECK(binaryBuffer == cBinaryData);
            CHECK(binaryBufferSize == sizeof(cBinaryData));
        }
    }

    SECTION("Success - Binary buffer is larger than needed") {
        result = foeDecodeHex(cUpperHexData.size(), cUpperHexData.data(), &binaryBufferSize,
                              &binaryBuffer);
        CHECK(result.value == FOE_SUCCESS);

        CHECK(binaryBuffer == cBinaryData);
        CHECK(binaryBufferSize == sizeof(cBinaryData));
    }

    SECTION("Failure - Binary buffer is too small") {
        binaryBufferSize = sizeof(cBinaryData) - 2;
        result = foeDecodeHex(cUpperHexData.size(), cUpperHexData.data(), &binaryBufferSize,
                              &binaryBuffer);
        CHECK(result.value == FOE_ERROR_DESTINATION_BUFFER_TOO_SMALL);
    }

    SECTION("Failure - Hex data size is not multiple of 2") {
        result = foeDecodeHex(1, cUpperHexData.data(), &binaryBufferSize, &binaryBuffer);
        CHECK(result.value == FOE_ERROR_INVALID_HEX_DATA_SIZE);

        result = foeDecodeHex(3, cUpperHexData.data(), &binaryBufferSize, &binaryBuffer);
        CHECK(result.value == FOE_ERROR_INVALID_HEX_DATA_SIZE);

        result = foeDecodeHex(5, cUpperHexData.data(), &binaryBufferSize, &binaryBuffer);
        CHECK(result.value == FOE_ERROR_INVALID_HEX_DATA_SIZE);

        result = foeDecodeHex(7, cUpperHexData.data(), &binaryBufferSize, &binaryBuffer);
        CHECK(result.value == FOE_ERROR_INVALID_HEX_DATA_SIZE);
    }

    SECTION("Failure - Hex data ends short of expected on odd byte") {
        std::string hexData = cUpperHexData;
        hexData[5] = '\0';

        REQUIRE(hexData.size() == cUpperHexData.size());

        result = foeDecodeHex(hexData.size(), hexData.data(), &binaryBufferSize, &binaryBuffer);
        CHECK(result.value == FOE_ERROR_MALFORMED_HEX_DATA);
    }

    SECTION("Failure - Hex data ends short of expected on even byte") {
        std::string hexData = cUpperHexData;
        hexData[4] = '\0';

        REQUIRE(hexData.size() == cUpperHexData.size());

        result = foeDecodeHex(hexData.size(), hexData.data(), &binaryBufferSize, &binaryBuffer);
        CHECK(result.value == FOE_ERROR_MALFORMED_HEX_DATA);
    }
}

TEST_CASE("Binary/Hex Encoding/Decoding - All possible single byte values (0-255)") {
    foeResultSet result;

    for (uint32_t i = 0; i <= UINT8_MAX; ++i) {
        std::array<char, 3> hexArr = {};
        result = foeEncodeHex(1, &i, hexArr.size(), hexArr.data());
        REQUIRE(result.value == FOE_SUCCESS);

        uint8_t decodedByte;
        size_t decodedDataSize = 1;
        result = foeDecodeHex(hexArr.size() - 1, hexArr.data(), &decodedDataSize, &decodedByte);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(decodedByte == i);
        REQUIRE(decodedDataSize == 1);
    }
}