/*
    Copyright (C) 2022 George Cave.

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
#include <foe/resource/create_info.h>
#include <foe/resource/error_code.h>

#include "test_log_sink.hpp"

#include <thread>

constexpr size_t cNumThreads = 4;
constexpr size_t cNumCount = 8192 * 8;

TEST_CASE("foeResourceCreateInfo - Create without a data function fails") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    foeErrorCode errC = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, nullptr, &createInfo);

    CHECK(errC.value == FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED);
    CHECK(createInfo == FOE_NULL_HANDLE);

    SECTION("Ensure the 'pCreateInfo' variable isn't updated on a failure case") {
        createInfo = (foeResourceCreateInfo)0xbebebebebebe;

        errC = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, nullptr, &createInfo);

        CHECK(errC.value == FOE_RESOURCE_ERROR_DATA_FUNCTION_NOT_PROVIDED);
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

    foeErrorCode errC = foeCreateResourceCreateInfo(0, destroyFn, sizeof(TestStruct), &testStruct,
                                                    testDataFn, &createInfo);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(createInfo != FOE_NULL_HANDLE);

    CHECK(!destroyCalled);
    foeDestroyResourceCreateInfo(createInfo);
    CHECK(destroyCalled);
}

TEST_CASE("foeResourceCreateInfo - Create properly sets initial state and different Type values") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    SECTION("Type: 0") {
        foeErrorCode errC =
            foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

        CHECK(errC.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetType(createInfo) == 0);
    }
    SECTION("Type: 1") {
        foeErrorCode errC =
            foeCreateResourceCreateInfo(1, nullptr, 0, nullptr, dummyData, &createInfo);

        CHECK(errC.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetType(createInfo) == 1);
    }
    SECTION("Type: UINT32_MAX") {
        foeErrorCode errC =
            foeCreateResourceCreateInfo(UINT32_MAX, nullptr, 0, nullptr, dummyData, &createInfo);

        CHECK(errC.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetType(createInfo) == UINT32_MAX);
    }

    foeDestroyResourceCreateInfo(createInfo);
}

TEST_CASE("foeResourceCreateInfo - Incrementing/Decrementing reference count") {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    foeErrorCode errC = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
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

    foeErrorCode errC = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
    CHECK(createInfo != FOE_NULL_HANDLE);

    foeDestroyResourceCreateInfo(createInfo);

    foeLogger::instance()->deregisterSink(&testSink);

    CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[0].msg.starts_with("foeResourceCreateInfo["));
    CHECK(testSink.logMessages[0].msg.ends_with(",0] - Created"));

    CHECK(testSink.logMessages[1].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[1].msg.starts_with("foeResourceCreateInfo["));
    CHECK(testSink.logMessages[1].msg.ends_with(",0] - Destroying..."));

    CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[2].msg.starts_with("foeResourceCreateInfo["));
    CHECK(testSink.logMessages[2].msg.ends_with(",0] - Destroyed"));
}

TEST_CASE("foeResourceCreateInfo - Warning logged when destroyed with non-zero reference count") {
    TestLogSink testSink;
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};
    auto dummyData = [](void *, void *) {};

    foeErrorCode errC = foeCreateResourceCreateInfo(0, nullptr, 0, nullptr, dummyData, &createInfo);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
    CHECK(createInfo != FOE_NULL_HANDLE);

    foeResourceCreateInfoIncrementRefCount(createInfo);
    foeResourceCreateInfoIncrementRefCount(createInfo);
    foeResourceCreateInfoIncrementRefCount(createInfo);

    foeLogger::instance()->registerSink(&testSink);

    foeDestroyResourceCreateInfo(createInfo);

    foeLogger::instance()->deregisterSink(&testSink);

    CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[0].msg.starts_with("foeResourceCreateInfo["));
    CHECK(testSink.logMessages[0].msg.ends_with("0] - Destroying..."));

    CHECK(testSink.logMessages[1].level == foeLogLevel::Warning);
    CHECK(testSink.logMessages[1].msg.starts_with("foeResourceCreateInfo["));
    CHECK(testSink.logMessages[1].msg.ends_with(
        "0] - Destroying with a non-zero reference count of: 3"));

    CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[2].msg.starts_with("foeResourceCreateInfo["));
    CHECK(testSink.logMessages[2].msg.ends_with("0] - Destroyed"));
}