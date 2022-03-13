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
#include <foe/resource/error_code.h>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.hpp>

#include "test_log_sink.hpp"

#include <thread>

constexpr size_t cNumThreads = 4;
constexpr size_t cNumCount = 8192 * 8;

TEST_CASE("foeResource - Create while not providing foeResourceFns fails") {
    foeResource resource{FOE_NULL_HANDLE};

    foeErrorCode errC = foeCreateResource(0, 0, nullptr, 0, &resource);

    CHECK(errC.value == FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED);
    CHECK(resource == FOE_NULL_HANDLE);

    SECTION("Ensure the 'pResource' variable isn't updated on a failure case") {
        resource = (foeResource)0xbebebebebebe;

        errC = foeCreateResource(0, 0, nullptr, 0, &resource);

        CHECK(errC.value == FOE_RESOURCE_ERROR_RESOURCE_FUNCTIONS_NOT_PROVIDED);
        CHECK(resource == (foeResource)0xbebebebebebe);
    }
}

TEST_CASE("foeResource - Create properly sets initial state and different Type/ID values") {
    foeResourceFns fns{};
    foeResource resource{FOE_NULL_HANDLE};

    SECTION("Type: 0 / ID: 0") {
        foeErrorCode errC = foeCreateResource(0, 0, &fns, 0, &resource);

        CHECK(errC.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);
        CHECK(foeResourceGetID(resource) == 0);
        CHECK(foeResourceGetType(resource) == 0);
    }
    SECTION("Type: 1 / ID: 1") {
        foeErrorCode errC = foeCreateResource(1, 1, &fns, 0, &resource);

        CHECK(errC.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);
        CHECK(foeResourceGetID(resource) == 1);
        CHECK(foeResourceGetType(resource) == 1);
    }
    SECTION("Type: UINT32_MAX / ID: UINT32_MAX") {
        foeErrorCode errC = foeCreateResource(UINT32_MAX, UINT32_MAX, &fns, 0, &resource);

        CHECK(errC.value == FOE_RESOURCE_SUCCESS);
        REQUIRE(resource != FOE_NULL_HANDLE);
        CHECK(foeResourceGetID(resource) == UINT32_MAX);
        CHECK(foeResourceGetType(resource) == UINT32_MAX);
    }

    CHECK(foeResourceGetCreateInfo(resource) == FOE_NULL_HANDLE);
    CHECK_FALSE(foeResourceGetIsLoading(resource));
    CHECK(foeResourceGetState(resource) == foeResourceLoadState::Unloaded);

    foeDestroyResource(resource);
}

TEST_CASE("foeResource - Incrementing/Decrementing reference and use counts") {
    foeResourceFns fns{};
    foeResource resource{FOE_NULL_HANDLE};

    foeErrorCode errC = foeCreateResource(0, 0, &fns, 0, &resource);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    SECTION("Single-threaded") {
        SECTION("Reference count") {
            CHECK(foeResourceIncrementRefCount(resource) == 1);
            CHECK(foeResourceGetRefCount(resource) == 1);

            for (int i = 1; i < 100; ++i) {
                CHECK(foeResourceIncrementRefCount(resource) == i + 1);
                CHECK(foeResourceGetRefCount(resource) == i + 1);
            }

            CHECK(foeResourceGetRefCount(resource) == 100);

            for (int i = 0; i < 99; ++i) {
                CHECK(foeResourceDecrementRefCount(resource) == 99 - i);
                CHECK(foeResourceGetRefCount(resource) == 99 - i);
            }

            CHECK(foeResourceDecrementRefCount(resource) == 0);
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

            CHECK(foeResourceGetRefCount(resource) == cNumThreads * cNumCount);

            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i] = std::thread(decrementFn);
            }
            for (size_t i = 0; i < cNumThreads; ++i) {
                threads[i].join();
            }

            CHECK(foeResourceGetRefCount(resource) == 0);
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

    foeDestroyResource(resource);
}

TEST_CASE("foeResource - Regular lifetime logs (no import/loading)") {
    TestLogSink testSink;
    foeResourceFns fns{};
    foeResource resource{FOE_NULL_HANDLE};

    foeLogger::instance()->registerSink(&testSink);

    foeErrorCode errC = foeCreateResource(0, 0, &fns, 0, &resource);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
    CHECK(resource != FOE_NULL_HANDLE);

    foeDestroyResource(resource);

    foeLogger::instance()->deregisterSink(&testSink);

    CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[0].msg.starts_with("foeResource["));
    CHECK(testSink.logMessages[0].msg.find(",0] - Created @ ") != std::string::npos);

    CHECK(testSink.logMessages[1].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[1].msg.starts_with("foeResource["));
    CHECK(testSink.logMessages[1].msg.ends_with(",0] - Destroying..."));

    CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
    CHECK(testSink.logMessages[2].msg.starts_with("foeResource["));
    CHECK(testSink.logMessages[2].msg.ends_with(",0] - Destroyed"));
}

TEST_CASE("foeResource - Warning logged when destroyed with non-zero reference or use count") {
    TestLogSink testSink;
    foeResourceFns fns{};
    foeResource resource{FOE_NULL_HANDLE};

    foeErrorCode errC = foeCreateResource(0, 0, &fns, 0, &resource);

    CHECK(errC.value == FOE_RESOURCE_SUCCESS);
    CHECK(resource != FOE_NULL_HANDLE);

    SECTION("Reference Count") {
        foeResourceIncrementRefCount(resource);

        foeLogger::instance()->registerSink(&testSink);

        foeDestroyResource(resource);

        foeLogger::instance()->deregisterSink(&testSink);

        CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
        CHECK(testSink.logMessages[0].msg == "foeResource[0,0] - Destroying...");

        CHECK(testSink.logMessages[1].level == foeLogLevel::Warning);
        CHECK(testSink.logMessages[1].msg ==
              "foeResource[0,0] - Destroying with a non-zero reference count of: 1");

        CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
        CHECK(testSink.logMessages[2].msg == "foeResource[0,0] - Destroyed");
    }
    SECTION("Use Count") {
        foeResourceIncrementUseCount(resource);
        foeResourceIncrementUseCount(resource);

        foeLogger::instance()->registerSink(&testSink);

        foeDestroyResource(resource);

        foeLogger::instance()->deregisterSink(&testSink);

        CHECK(testSink.logMessages[0].level == foeLogLevel::Verbose);
        CHECK(testSink.logMessages[0].msg == "foeResource[0,0] - Destroying...");

        CHECK(testSink.logMessages[1].level == foeLogLevel::Warning);
        CHECK(testSink.logMessages[1].msg ==
              "foeResource[0,0] - Destroying with a non-zero use count of: 2");

        CHECK(testSink.logMessages[2].level == foeLogLevel::Verbose);
        CHECK(testSink.logMessages[2].msg == "foeResource[0,0] - Destroyed");
    }
}