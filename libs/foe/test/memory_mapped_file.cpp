// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch2/catch_test_macros.hpp>
#include <foe/memory_mapped_file.h>
#include <foe/result.h>

TEST_CASE("MemoryMappedFile - Success Cases") {
    foeManagedMemory test = FOE_NULL_HANDLE;
    void *pData = nullptr;
    uint32_t dataSize = UINT32_MAX;

    SECTION("4KB file") {
        foeResultSet result =
            foeCreateMemoryMappedFile(TEST_DIR "/data/memory_mapped_file/4kb_file", &test);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(test != FOE_NULL_HANDLE);

        CHECK(foeManagedMemoryGetUse(test) == 1);

        foeManagedMemoryGetData(test, &pData, &dataSize);
        CHECK(pData != nullptr);
        CHECK(dataSize == 4 * 1024);
    }

    SECTION("4KB file") {
        foeResultSet result =
            foeCreateMemoryMappedFile(TEST_DIR "/data/memory_mapped_file/128kb_file", &test);
        REQUIRE(result.value == FOE_SUCCESS);
        REQUIRE(test != FOE_NULL_HANDLE);

        CHECK(foeManagedMemoryGetUse(test) == 1);

        foeManagedMemoryGetData(test, &pData, &dataSize);
        CHECK(pData != nullptr);
        CHECK(dataSize == 128 * 1024);
    }

    CHECK(foeManagedMemoryIncrementUse(test) == 2);
    CHECK(foeManagedMemoryDecrementUse(test) == 1);
    CHECK(foeManagedMemoryDecrementUse(test) == 0);
}

TEST_CASE("MemoryMappedFile - Failure Cases") {
    foeManagedMemory test = FOE_NULL_HANDLE;

    SECTION("Attempting to open a non-existing file") {
        foeResultSet result =
            foeCreateMemoryMappedFile(TEST_DIR "/data/memory_mapped_file/non_existing_file", &test);
        REQUIRE(result.value == FOE_ERROR_FAILED_TO_OPEN_FILE);
        REQUIRE(test == FOE_NULL_HANDLE);
    }
    SECTION("Attempting to open a zero-sized file") {
        foeResultSet result =
            foeCreateMemoryMappedFile(TEST_DIR "/data/memory_mapped_file/0b_file", &test);
        REQUIRE(result.value == FOE_ERROR_ATTEMPTED_TO_MAP_ZERO_SIZED_FILE);
        REQUIRE(test == FOE_NULL_HANDLE);
    }
}