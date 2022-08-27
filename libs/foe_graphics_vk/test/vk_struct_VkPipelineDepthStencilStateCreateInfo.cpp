// Copyright (C) 2020-2022 George Cave - gcave@stablecoder.ca
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/binary_result.h>
#include <foe/graphics/vk/vk_binary.h>
#include <vk_struct_compare.h>

namespace {

constexpr VkPipelineDepthStencilStateCreateInfo cFilledData{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthCompareOp = VK_COMPARE_OP_GREATER,
    .front =
        VkStencilOpState{
            .failOp = VK_STENCIL_OP_INVERT,
            .writeMask = 12,
        },
    .back =
        VkStencilOpState{
            .failOp = VK_STENCIL_OP_REPLACE,
            .writeMask = 10,
        },
    .maxDepthBounds = 1024.5f,
};

} // namespace

TEST_CASE("binary read/write for VkPipelineDepthStencilStateCreateInfo", "[foe][vulkan]") {
    foeResultSet resultSet;
    std::unique_ptr<std::byte[]> pRaw;
    uint32_t requiredSize = 0;

    SECTION("Zero-filled data struct") {
        VkPipelineDepthStencilStateCreateInfo writeData{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };

        resultSet =
            binary_write_VkPipelineDepthStencilStateCreateInfo(&writeData, &requiredSize, nullptr);

        REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
        CHECK(requiredSize != 0);

        SECTION("Attempting to write to smaller than required sized buffer fails") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = 0;

            resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&writeData, &writeSize,
                                                                           pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        }
        SECTION("Write to exact-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = requiredSize;

            resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&writeData, &writeSize,
                                                                           pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Reading back from under-sized buffer fails") {
                for (uint32_t i = 0; i < writeSize; ++i) {
                    VkPipelineDepthStencilStateCreateInfo readData;
                    uint32_t readSize = i;

                    resultSet = binary_read_VkPipelineDepthStencilStateCreateInfo(
                        pRaw.get(), &readSize, &readData);

                    REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                }
            }
            SECTION("Read data back from exact-sized written buffer") {
                VkPipelineDepthStencilStateCreateInfo readData;
                uint32_t readSize = writeSize;

                resultSet = binary_read_VkPipelineDepthStencilStateCreateInfo(pRaw.get(), &readSize,
                                                                              &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkPipelineDepthStencilStateCreateInfo(&writeData, &readData));
            }
        }
        SECTION("Write to over-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize + 1024]);
            uint32_t writeSize = requiredSize + 1024;

            resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&writeData, &writeSize,
                                                                           pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Read data back from over-sized written buffer") {
                VkPipelineDepthStencilStateCreateInfo readData;
                uint32_t readSize = writeSize + 512;

                resultSet = binary_read_VkPipelineDepthStencilStateCreateInfo(pRaw.get(), &readSize,
                                                                              &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkPipelineDepthStencilStateCreateInfo(&writeData, &readData));
            }
        }
    }
    SECTION("Custom-filled data struct") {
        resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&cFilledData, &requiredSize,
                                                                       nullptr);

        REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
        CHECK(requiredSize != 0);

        SECTION("Attempting to write to smaller than required sized buffer fails") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = 0;

            resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&cFilledData, &writeSize,
                                                                           pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
        }
        SECTION("Write to exact-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize]);
            uint32_t writeSize = requiredSize;

            resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&cFilledData, &writeSize,
                                                                           pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Reading back from under-sized buffer fails") {
                for (uint32_t i = 0; i < writeSize; ++i) {
                    VkPipelineDepthStencilStateCreateInfo readData;
                    uint32_t readSize = i;

                    resultSet = binary_read_VkPipelineDepthStencilStateCreateInfo(
                        pRaw.get(), &readSize, &readData);

                    REQUIRE(resultSet.value == FOE_BINARY_ERROR_INSUFFICIENT_BUFFER_SIZE);
                }
            }
            SECTION("Read data back from exact-sized written buffer") {
                VkPipelineDepthStencilStateCreateInfo readData;
                uint32_t readSize = writeSize;

                resultSet = binary_read_VkPipelineDepthStencilStateCreateInfo(pRaw.get(), &readSize,
                                                                              &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkPipelineDepthStencilStateCreateInfo(&cFilledData, &readData));
            }
        }
        SECTION("Write to over-sized buffer successfully") {
            pRaw.reset(new std::byte[requiredSize + 1024]);
            uint32_t writeSize = requiredSize + 1024;

            resultSet = binary_write_VkPipelineDepthStencilStateCreateInfo(&cFilledData, &writeSize,
                                                                           pRaw.get());

            REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
            CHECK(writeSize == requiredSize);

            SECTION("Read data back from over-sized written buffer") {
                VkPipelineDepthStencilStateCreateInfo readData;
                uint32_t readSize = writeSize + 512;

                resultSet = binary_read_VkPipelineDepthStencilStateCreateInfo(pRaw.get(), &readSize,
                                                                              &readData);

                REQUIRE(resultSet.value == FOE_BINARY_SUCCESS);
                CHECK(readSize == writeSize);
                CHECK(compare_VkPipelineDepthStencilStateCreateInfo(&cFilledData, &readData));
            }
        }
    }
}
