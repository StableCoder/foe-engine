// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/managed_memory.h>
#include <foe/result.h>

constexpr size_t cDataCount = 2048;

TEST_CASE("ManagedMemory - No metadata/cleanup") {
    foeManagedMemory managedMemory = FOE_NULL_HANDLE;
    foeResultSet result;

    uint16_t buffer[cDataCount];
    for (int i = 0; i < cDataCount; ++i) {
        buffer[i] = i;
    }

    result = foeCreateManagedMemory(buffer, sizeof(uint16_t) * cDataCount, nullptr, nullptr, 0,
                                    &managedMemory);
    CHECK(result.value == FOE_SUCCESS);
    REQUIRE(managedMemory != FOE_NULL_HANDLE);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 1);

    uint16_t *ptr = nullptr;
    uint32_t size = 0;

    foeManagedMemoryGetData(managedMemory, (void **)&ptr, &size);
    REQUIRE(ptr == buffer);
    REQUIRE(size == cDataCount * sizeof(uint16_t));

    for (uint16_t i = 0; i < cDataCount; ++i) {
        if (ptr[i] != i)
            CHECK(ptr[i] == i);
    }

    CHECK(foeManagedMemoryIncrementUse(managedMemory) == 2);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 2);
    CHECK(foeManagedMemoryDecrementUse(managedMemory) == 1);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 1);

    REQUIRE(foeManagedMemoryDecrementUse(managedMemory) == 0);
}

TEST_CASE("ManagedMemory - metadata and cleanup") {
    foeManagedMemory managedMemory = FOE_NULL_HANDLE;
    foeResultSet result;

    uint16_t *buffer = new uint16_t[cDataCount];
    for (int i = 0; i < cDataCount; ++i) {
        buffer[i] = i;
    }

    auto cleanupFn = [](void *pData, uint32_t, void *pMetadata) {
        uint16_t **ppData = (uint16_t **)pMetadata;
        CHECK(*ppData == pData);

        delete[](uint16_t *) pData;
    };

    result = foeCreateManagedMemory(buffer, sizeof(uint16_t) * cDataCount, cleanupFn, &buffer,
                                    sizeof(uint16_t *), &managedMemory);
    CHECK(result.value == FOE_SUCCESS);
    REQUIRE(managedMemory != FOE_NULL_HANDLE);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 1);

    uint16_t *ptr = nullptr;
    uint32_t size = 0;

    foeManagedMemoryGetData(managedMemory, (void **)&ptr, &size);
    REQUIRE(ptr == buffer);
    REQUIRE(size == cDataCount * sizeof(uint16_t));

    for (uint16_t i = 0; i < cDataCount; ++i) {
        if (ptr[i] != i)
            CHECK(ptr[i] == i);
    }

    CHECK(foeManagedMemoryIncrementUse(managedMemory) == 2);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 2);
    CHECK(foeManagedMemoryDecrementUse(managedMemory) == 1);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 1);

    REQUIRE(foeManagedMemoryDecrementUse(managedMemory) == 0);
}

TEST_CASE("ManagedMemory - subset success case") {
    foeManagedMemory managedMemory = FOE_NULL_HANDLE;
    foeManagedMemory managedSubset = FOE_NULL_HANDLE;
    foeResultSet result;

    uint16_t *buffer = new uint16_t[cDataCount];
    for (int i = 0; i < cDataCount; ++i) {
        buffer[i] = i;
    }

    auto cleanupFn = [](void *pData, uint32_t, void *pMetadata) {
        uint16_t **ppData = (uint16_t **)pMetadata;
        CHECK(*ppData == pData);

        delete[](uint16_t *) pData;
    };

    result = foeCreateManagedMemory(buffer, sizeof(uint16_t) * cDataCount, cleanupFn, &buffer,
                                    sizeof(uint16_t *), &managedMemory);
    CHECK(result.value == FOE_SUCCESS);
    REQUIRE(managedMemory != FOE_NULL_HANDLE);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 1);

    result = foeCreateManagedMemorySubset(managedMemory, (cDataCount / 2) * sizeof(uint16_t),
                                          (cDataCount / 2) * sizeof(uint16_t), &managedSubset);
    CHECK(result.value == FOE_SUCCESS);
    REQUIRE(managedSubset != FOE_NULL_HANDLE);
    CHECK(foeManagedMemoryGetUse(managedSubset) == 1);

    // Original memory has an increased use
    CHECK(foeManagedMemoryDecrementUse(managedMemory) > 0);

    uint16_t *ptr = nullptr;
    uint32_t size = 0;

    foeManagedMemoryGetData(managedSubset, (void **)&ptr, &size);
    REQUIRE(ptr == buffer + (cDataCount / 2));
    REQUIRE(size == (cDataCount / 2) * sizeof(uint16_t));

    for (uint16_t i = 0; i < cDataCount / 2; ++i) {
        if (ptr[i] != (cDataCount / 2) + i)
            CHECK(ptr[i] == (cDataCount / 2) + i);
    }

    CHECK(foeManagedMemoryIncrementUse(managedSubset) == 2);
    CHECK(foeManagedMemoryGetUse(managedSubset) == 2);
    CHECK(foeManagedMemoryDecrementUse(managedSubset) == 1);
    CHECK(foeManagedMemoryGetUse(managedSubset) == 1);

    REQUIRE(foeManagedMemoryDecrementUse(managedSubset) == 0);
}

TEST_CASE("ManagedMemory - subset failure case") {
    foeManagedMemory managedMemory = FOE_NULL_HANDLE;
    foeManagedMemory managedSubset = FOE_NULL_HANDLE;
    foeResultSet result;

    uint16_t *buffer = new uint16_t[cDataCount];
    for (int i = 0; i < cDataCount; ++i) {
        buffer[i] = i;
    }

    auto cleanupFn = [](void *pData, uint32_t, void *pMetadata) {
        uint16_t **ppData = (uint16_t **)pMetadata;
        CHECK(*ppData == pData);

        delete[](uint16_t *) pData;
    };

    result = foeCreateManagedMemory(buffer, sizeof(uint16_t) * cDataCount, cleanupFn, &buffer,
                                    sizeof(uint16_t *), &managedMemory);
    CHECK(result.value == FOE_SUCCESS);
    REQUIRE(managedMemory != FOE_NULL_HANDLE);
    CHECK(foeManagedMemoryGetUse(managedMemory) == 1);

    result = foeCreateManagedMemorySubset(managedMemory, (cDataCount / 2) * sizeof(uint16_t),
                                          (cDataCount / 2) * sizeof(uint16_t) + 1, &managedSubset);
    CHECK(result.value == FOE_ERROR_MEMORY_SUBSET_OVERRUNS_PARENT);
    REQUIRE(managedSubset == FOE_NULL_HANDLE);

    REQUIRE(foeManagedMemoryDecrementUse(managedMemory) == 0);
}
