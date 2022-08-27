// Copyright (C) 2020-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/binary_result.h>
#include <foe/graphics/resource/binary.h>
#include <foe/graphics/resource/cleanup.h>
#include <foe/graphics/resource/compare.h>
#include <foe/graphics/resource/image_create_info.h>

namespace {

constexpr foeImageCreateInfo cFilledData{.pFile = "example.png"};

} // namespace

TEST_CASE("binary read/write for foeImageCreateInfo", "[foe][vulkan]") {
    foeResultSet resultSet;
    std::unique_ptr<std::byte[]> pRaw;
    uint32_t requiredSize = 0;

    SECTION("Zero-filled data struct") {
        foeImageCreateInfo writeData = {};

        resultSet = binary_write_foeImageCreateInfo(&writeData, &requiredSize, nullptr);

        REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
        CHECK(requiredSize != 0);

        SECTION("Attempting to write to smaller than required sized buffer fails") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = 0;

            resultSet = binary_write_foeImageCreateInfo(&writeData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        }
        SECTION("Write to exact-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = requiredSize;

            resultSet = binary_write_foeImageCreateInfo(&writeData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Reading back from under-sized buffer fails") {
                for (uint32_t i = 0; i < writeSize; ++i) {
                    foeImageCreateInfo readData;
                    uint32_t readSize = i;

                    resultSet = binary_read_foeImageCreateInfo(pRaw.get(), &readSize, &readData);

                    REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                }
            }
            SECTION("Read data back from exact-sized written buffer") {
                foeImageCreateInfo readData;
                uint32_t readSize = writeSize;

                resultSet = binary_read_foeImageCreateInfo(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_foeImageCreateInfo(&writeData, &readData));
            }
        }
        SECTION("Write to over-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize + 1024]);
            uint32_t writeSize = requiredSize + 1024;

            resultSet = binary_write_foeImageCreateInfo(&writeData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Read data back from over-sized written buffer") {
                foeImageCreateInfo readData;
                uint32_t readSize = writeSize + 512;

                resultSet = binary_read_foeImageCreateInfo(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_foeImageCreateInfo(&writeData, &readData));
            }
        }
    }
    SECTION("Custom-filled data struct") {
        resultSet = binary_write_foeImageCreateInfo(&cFilledData, &requiredSize, nullptr);

        REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
        CHECK(requiredSize != 0);

        SECTION("Attempting to write to smaller than required sized buffer fails") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = 0;

            resultSet = binary_write_foeImageCreateInfo(&cFilledData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        }
        SECTION("Write to exact-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = requiredSize;

            resultSet = binary_write_foeImageCreateInfo(&cFilledData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Reading back from under-sized buffer fails") {
                for (uint32_t i = 0; i < writeSize; ++i) {
                    foeImageCreateInfo readData;
                    uint32_t readSize = i;

                    resultSet = binary_read_foeImageCreateInfo(pRaw.get(), &readSize, &readData);

                    REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                }
            }
            SECTION("Read data back from exact-sized written buffer") {
                foeImageCreateInfo readData;
                uint32_t readSize = writeSize;

                resultSet = binary_read_foeImageCreateInfo(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_foeImageCreateInfo(&cFilledData, &readData));

                cleanup_foeImageCreateInfo(&readData);
            }
        }
        SECTION("Write to over-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize + 1024]);
            uint32_t writeSize = requiredSize + 1024;

            resultSet = binary_write_foeImageCreateInfo(&cFilledData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Read data back from over-sized written buffer") {
                foeImageCreateInfo readData;
                uint32_t readSize = writeSize + 512;

                resultSet = binary_read_foeImageCreateInfo(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_foeImageCreateInfo(&cFilledData, &readData));

                cleanup_foeImageCreateInfo(&readData);
            }
        }
    }
}
