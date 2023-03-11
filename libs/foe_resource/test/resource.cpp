// Copyright (C) 2022-2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.h>
#include <foe/resource/result.h>

#include "test_log_sink.hpp"

#include <thread>

constexpr size_t cNumThreads = 4;
constexpr size_t cNumCount = 8192 * 8;

TEST_CASE("foeResource - Create while not providing foeResourceFns fails") {
    foeResource resource{(foeResource)0xdeadbeef};

    foeResultSet result = foeCreateResource(0, 0, nullptr, sizeof(foeResourceBase), &resource);

    CHECK(result.value == FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED);
    CHECK(resource == (foeResource)0xdeadbeef);
}

TEST_CASE("foeResource - Create when data size is < sizeof(foeResourceBase) fails") {
    foeResource resource{(foeResource)0xdeadbeef};
    foeResourceFns fns{};
    foeResultSet result;

    SECTION("Size of 0") {
        result = foeCreateResource(0, 0, &fns, 0, &resource);
        CHECK(result.value == FOE_RESOURCE_ERROR_DATA_SIZE_SMALLER_THAN_BASE);
        CHECK(resource == (foeResource)0xdeadbeef);
    }
    SECTION("One byte smaller") {
        result = foeCreateResource(0, 0, &fns, sizeof(foeResourceBase) - 1, &resource);
        CHECK(result.value == FOE_RESOURCE_ERROR_DATA_SIZE_SMALLER_THAN_BASE);
        CHECK(resource == (foeResource)0xdeadbeef);
    }
}

TEST_CASE("foeResource - Create properly sets initial state and different Type/ID values") {
    foeResourceFns fns{};
    foeResource resource{FOE_NULL_HANDLE};

    SECTION("Type: 0 / ID: 0") {
        foeResultSet result = foeCreateResource(0, 0, &fns, sizeof(foeResourceBase), &resource);

        CHECK(result.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);
        CHECK(foeResourceGetID(resource) == 0);
        CHECK(foeResourceGetType(resource) == 0);
        REQUIRE(foeResourceGetRefCount(resource) == 1);
    }
    SECTION("Type: 1 / ID: 1") {
        foeResultSet result = foeCreateResource(1, 1, &fns, sizeof(foeResourceBase), &resource);

        CHECK(result.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);
        CHECK(foeResourceGetID(resource) == 1);
        CHECK(foeResourceGetType(resource) == 1);
        REQUIRE(foeResourceGetRefCount(resource) == 1);
    }
    SECTION("Type: INT_MAX / ID: UINT32_MAX") {
        foeResultSet result =
            foeCreateResource(UINT32_MAX, INT_MAX, &fns, sizeof(foeResourceBase), &resource);

        CHECK(result.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);
        CHECK(foeResourceGetID(resource) == UINT32_MAX);
        CHECK(foeResourceGetType(resource) == INT_MAX);
        REQUIRE(foeResourceGetRefCount(resource) == 1);
    }

    CHECK_FALSE(foeResourceGetIsLoading(resource));
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);

    CHECK(foeResourceDecrementRefCount(resource) == 0);
}

TEST_CASE("foeResource - Incrementing/Decrementing reference and use counts") {
    foeResourceFns fns{};
    foeResource resource{FOE_NULL_HANDLE};

    foeResultSet result = foeCreateResource(0, 0, &fns, sizeof(foeResourceBase), &resource);

    REQUIRE(result.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);
    REQUIRE(foeResourceGetRefCount(resource) == 1);

    SECTION("Single-threaded") {
        SECTION("Reference count") {
            for (int i = 1; i < 100; ++i) {
                CHECK(foeResourceIncrementRefCount(resource) == i + 1);
                CHECK(foeResourceGetRefCount(resource) == i + 1);
            }

            CHECK(foeResourceGetRefCount(resource) == 100);

            for (int i = 0; i < 99; ++i) {
                CHECK(foeResourceDecrementRefCount(resource) == 99 - i);
                CHECK(foeResourceGetRefCount(resource) == 99 - i);
            }
        }
        SECTION("Use count") {
            CHECK(foeResourceIncrementUseCount(resource) == 1);
            CHECK(foeResourceGetUseCount(resource) == 1);

            for (int i = 1; i < 100; ++i) {
                CHECK(foeResourceIncrementUseCount(resource) == i + 1);
                CHECK(foeResourceGetUseCount(resource) == i + 1);
            }

            CHECK(foeResourceGetUseCount(resource) == 100);

            for (int i = 0; i < 99; ++i) {
                CHECK(foeResourceDecrementUseCount(resource) == 99 - i);
                CHECK(foeResourceGetUseCount(resource) == 99 - i);
            }

            CHECK(foeResourceDecrementUseCount(resource) == 0);
        }
    }

    SECTION("Multi-threaded (must be thread-safe)") {
        SECTION("Reference count") {
            std::thread threads[cNumThreads];
            auto incrementFn = [resource]() {
                for (size_t i = 0; i < cNumCount; ++i) {
                    foeResourceIncrementRefCount(resource);
                }
            };
            auto decrementFn = [resource]() {
                for (size_t i = 0; i < cNumCount; ++i) {
                    foeResourceDecrementRefCount(resource);
                }
            };

            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i] = std::thread(incrementFn);
            }
            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i].join();
            }

            CHECK(foeResourceGetRefCount(resource) == 1 + cNumThreads * cNumCount);

            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i] = std::thread(decrementFn);
            }
            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i].join();
            }
        }
        SECTION("Use count") {
            std::thread threads[cNumThreads];
            auto incrementFn = [resource]() {
                for (size_t i = 0; i < cNumCount; ++i) {
                    foeResourceIncrementUseCount(resource);
                }
            };
            auto decrementFn = [resource]() {
                for (size_t i = 0; i < cNumCount; ++i) {
                    foeResourceDecrementUseCount(resource);
                }
            };

            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i] = std::thread(incrementFn);
            }
            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i].join();
            }

            CHECK(foeResourceGetUseCount(resource) == cNumThreads * cNumCount);

            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i] = std::thread(decrementFn);
            }
            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i].join();
            }

            CHECK(foeResourceGetUseCount(resource) == 0);
        }
    }

    CHECK(foeResourceDecrementRefCount(resource) == 0);
}