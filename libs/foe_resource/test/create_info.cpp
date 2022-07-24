// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/create_info.h>
#include <foe/resource/error_code.h>

#include "test_log_sink.hpp"

#include <thread>

constexpr size_t cNumThreads = 4;
constexpr size_t cNumCount = 8192 * 8;

TEST_CASE("foeResourceCreateInfo - Create without a data function fails") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    foeResultSet result = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, nullptr, &createInfo);

    CHECK(result.value == FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED);
    CHECK(createInfo == FOE_NULL_HANDLE);

    SECTION("Ensure the 'pCreateInfo' variable isn't updated on a failure case") {
        createInfo = (foeResourceCreateInfo)0xbebebebebebe;

        result = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, nullptr, &createInfo);

        CHECK(result.value == FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED);
        CHECK(createInfo == (foeResourceCreateInfo)0xbebebebebebe);
    }
}

TEST_CASE("foeResourceCreateInfo - Check that given destroy function is called on destroy") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    struct TestStruct {
        bool *pBool;
    };

    bool destroyCalled{false};
    TestStruct testStruct{.pBool = &destroyCalled};
    auto testDataFn = [](void *pSrc, void *pDst) {
        auto *pSrcData = (TestStruct *)pSrc;
        new (pDst) TestStruct(std::move(*pSrcData));
    };
    auto destroyFn = [](foeResourceCreateInfoType, void *pData) {
        auto *pTestStruct = (TestStruct *)pData;
        *pTestStruct->pBool = true;
    };

    foeResultSet result = foeCreateResourceCreateInfo(0, destroyFn, sizeof(TestStruct), &testStruct,
                                                      testDataFn, &createInfo);

    CHECK(result.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(createInfo != FOE_NULL_HANDLE);

    CHECK(!destroyCalled);
    foeDestroyResourceCreateInfo(createInfo);
    CHECK(destroyCalled);
}

TEST_CASE("foeResourceCreateInfo - Create properly sets initial state and different Type values") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    SECTION("Type: 0") {
        foeResultSet result =
            foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

        CHECK(result.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetType(createInfo) == 0);
    }
    SECTION("Type: 1") {
        foeResultSet result =
            foeCreateResourceCreateInfo(1, nullptr, 0, nullptr, dummyData, &createInfo);

        CHECK(result.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetType(createInfo) == 1);
    }
    SECTION("Type: UINT32_MAX") {
        foeResultSet result =
            foeCreateResourceCreateInfo(UINT32_MAX, nullptr, 0, nullptr, dummyData, &createInfo);

        CHECK(result.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetType(createInfo) == UINT32_MAX);
    }

    foeDestroyResourceCreateInfo(createInfo);
}

TEST_CASE("foeResourceCreateInfo - Incrementing/Decrementing reference count") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    foeResultSet result =
        foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

    CHECK(result.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(createInfo != FOE_NULL_HANDLE);

    SECTION("Single-threaded") {
        CHECK(foeResourceCreateInfoIncrementRefCount(createInfo) == 1);
        CHECK(foeResourceCreateInfoGetRefCount(createInfo) == 1);

        for (int i = 1; i < 100; ++i) {
            CHECK(foeResourceCreateInfoIncrementRefCount(createInfo) == i + 1);
            CHECK(foeResourceCreateInfoGetRefCount(createInfo) == i + 1);
        }

        CHECK(foeResourceCreateInfoGetRefCount(createInfo) == 100);

        for (int i = 0; i < 99; ++i) {
            CHECK(foeResourceCreateInfoDecrementRefCount(createInfo) == 99 - i);
            CHECK(foeResourceCreateInfoGetRefCount(createInfo) == 99 - i);
        }

        CHECK(foeResourceCreateInfoDecrementRefCount(createInfo) == 0);
    }

    SECTION("Multi-threaded (must be thread-safe)") {
        std::thread threads[cNumThreads];
        auto incrementFn = [createInfo]() {
            for (size_t i = 0; i < cNumCount; ++i) {
                foeResourceCreateInfoIncrementRefCount(createInfo);
            }
        };
        auto decrementFn = [createInfo]() {
            for (size_t i = 0; i < cNumCount; ++i) {
                foeResourceCreateInfoDecrementRefCount(createInfo);
            }
        };

        for (size_t i = 0; i < cNumThreads; ++i) {
            threads[i] = std::thread(incrementFn);
        }
        for (size_t i = 0; i < cNumThreads; ++i) {
            threads[i].join();
        }

        CHECK(foeResourceCreateInfoGetRefCount(createInfo) == cNumThreads * cNumCount);

        for (size_t i = 0; i < cNumThreads; ++i) {
            threads[i] = std::thread(decrementFn);
        }
        for (size_t i = 0; i < cNumThreads; ++i) {
            threads[i].join();
        }

        CHECK(foeResourceCreateInfoGetRefCount(createInfo) == 0);
    }

    foeDestroyResourceCreateInfo(createInfo);
}

TEST_CASE("foeResourceCreateInfo - Regular lifetime logs") {
    TestLogSink testSink;
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    foeLogger::instance()->registerSink(&testSink);

    foeResultSet result =
        foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

    CHECK(result.value == FOE_RESOURCE_SUCCESS);
    CHECK(createInfo != FOE_NULL_HANDLE);

    foeDestroyResourceCreateInfo(createInfo);
    foeLogger::instance()->deregisterSink(&testSink);

    REQUIRE(testSink.logMessages.size() == 3);

    CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[0].msg.starts_with("["));
    CHECK(testSink.logMessages[0].msg.ends_with(",0] foeResourceCreateInfo - Created"));

    CHECK(testSink.logMessages[1].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[1].msg.starts_with("["));
    CHECK(testSink.logMessages[1].msg.ends_with(",0] foeResourceCreateInfo - Destroying"));

    CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[2].msg.starts_with("["));
    CHECK(testSink.logMessages[2].msg.ends_with(",0] foeResourceCreateInfo - Destroyed"));
}

TEST_CASE("foeResourceCreateInfo - Warning logged when destroyed with non-zero reference count") {
    TestLogSink testSink;
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    foeResultSet result =
        foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

    CHECK(result.value == FOE_RESOURCE_SUCCESS);
    CHECK(createInfo != FOE_NULL_HANDLE);

    foeResourceCreateInfoIncrementRefCount(createInfo);
    foeResourceCreateInfoIncrementRefCount(createInfo);
    foeResourceCreateInfoIncrementRefCount(createInfo);

    foeLogger::instance()->registerSink(&testSink);

    foeDestroyResourceCreateInfo(createInfo);

    foeLogger::instance()->deregisterSink(&testSink);

    CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[0].msg.starts_with("["));
    CHECK(testSink.logMessages[0].msg.ends_with("0] foeResourceCreateInfo - Destroying"));

    CHECK(testSink.logMessages[1].level == foeLogLevel::Warning);
    CHECK(testSink.logMessages[1].msg.starts_with("["));
    CHECK(testSink.logMessages[1].msg.ends_with(
        "0] foeResourceCreateInfo - Destroying with a non-zero reference count of: 3"));

    CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[2].msg.starts_with("["));
    CHECK(testSink.logMessages[2].msg.ends_with("0] foeResourceCreateInfo - Destroyed"));
}