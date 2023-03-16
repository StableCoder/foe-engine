// Copyright (C) 2020-2023 George Cave
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/binary_result.h>
#include <foe/graphics/vk/vk_binary.h>
#include <vk_struct_compare.h>

#include <memory>

namespace {

constexpr VkStencilOpState cFilledData{
    .failOp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
    .depthFailOp = VK_STENCIL_OP_REPLACE,
    .compareMask = 152,
    .reference = 12,
};

}

TEST_CASE("binary read/write for VkStencilOpState", "[foe][vulkan]") {
    foeResultSet resultSet;
    std::unique_ptr<std::byte[]> pRaw;
    uint32_t requiredSize = 0;

    SECTION("Zero-filled data struct") {
        VkStencilOpState writeData = {};

        resultSet = binary_write_VkStencilOpState(&writeData, &requiredSize, nullptr);

        REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
        CHECK(requiredSize != 0);

        SECTION("Attempting to write to smaller than required sized buffer fails") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = 0;

            resultSet = binary_write_VkStencilOpState(&writeData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        }
        SECTION("Write to exact-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = requiredSize;

            resultSet = binary_write_VkStencilOpState(&writeData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Reading back from under-sized buffer fails") {
                for (uint32_t i = 0; i < writeSize; ++i) {
                    VkStencilOpState readData;
                    uint32_t readSize = i;

                    resultSet = binary_read_VkStencilOpState(pRaw.get(), &readSize, &readData);

                    REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                }
            }
            SECTION("Read data back from exact-sized written buffer") {
                VkStencilOpState readData;
                uint32_t readSize = writeSize;

                resultSet = binary_read_VkStencilOpState(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkStencilOpState(&writeData, &readData));
            }
        }
        SECTION("Write to over-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize + 1024]);
            uint32_t writeSize = requiredSize + 1024;

            resultSet = binary_write_VkStencilOpState(&writeData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Read data back from over-sized written buffer") {
                VkStencilOpState readData;
                uint32_t readSize = writeSize + 512;

                resultSet = binary_read_VkStencilOpState(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkStencilOpState(&writeData, &readData));
            }
        }
    }
    SECTION("Custom-filled data struct") {
        resultSet = binary_write_VkStencilOpState(&cFilledData, &requiredSize, nullptr);

        REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
        CHECK(requiredSize != 0);

        SECTION("Attempting to write to smaller than required sized buffer fails") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = 0;

            resultSet = binary_write_VkStencilOpState(&cFilledData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        }
        SECTION("Write to exact-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = requiredSize;

            resultSet = binary_write_VkStencilOpState(&cFilledData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Fails to read from undersized buffer") {
                VkStencilOpState readData;
                uint32_t readSize = writeSize / 2;

                resultSet = binary_read_VkStencilOpState(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
            }
            SECTION("Read data back from exact-sized written buffer") {
                VkStencilOpState readData;
                uint32_t readSize = writeSize;

                resultSet = binary_read_VkStencilOpState(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkStencilOpState(&cFilledData, &readData));
            }
        }
        SECTION("Write to over-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize + 1024]);
            uint32_t writeSize = requiredSize + 1024;

            resultSet = binary_write_VkStencilOpState(&cFilledData, &writeSize, pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Read data back from over-sized written buffer") {
                VkStencilOpState readData;
                uint32_t readSize = writeSize + 512;

                resultSet = binary_read_VkStencilOpState(pRaw.get(), &readSize, &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkStencilOpState(&cFilledData, &readData));
            }
        }
    }
}
