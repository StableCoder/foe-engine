// Copyright (C) 2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/error_code.h>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.h>
#include <foe/thread_pool.hpp>

#include "test_log_sink.hpp"

#include <atomic>

static std::atomic_int gGlobalIterator = 0;
constexpr foeResourceCreateInfoType cCreateInfoType = 1024;
constexpr foeResourceType cResourceType = 2048;

struct TestData {
    int value;
};

foeResourceCreateInfo genCreateInfoFn(void *, foeResourceID) {
    foeResourceCreateInfo createInfo{FOE_NULL_HANDLE};

    auto dataFn = [](void *, void *pDst) {
        auto *pData = (TestData *)pDst;
        pData->value = gGlobalIterator++;
    };

    foeResult result = foeCreateResourceCreateInfo(cCreateInfoType, nullptr, sizeof(TestData),
                                                   nullptr, dataFn, &createInfo);
    REQUIRE(result.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(createInfo != FOE_NULL_HANDLE);

    return createInfo;
}

TEST_CASE("foeResource - Importing CreateInfo") {
    foeResourceFns fns{
        .pImportFn = genCreateInfoFn,
    };

    foeResource resource{FOE_NULL_HANDLE};

    foeResult result = foeCreateResource(0, cResourceType, &fns, sizeof(TestData), &resource);
    REQUIRE(result.value == FOE_RESOURCE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    CHECK(foeResourceGetCreateInfo(resource) == FOE_NULL_HANDLE);

    SECTION("Import synchronously") {
        gGlobalIterator = 0;

        foeResourceImportCreateInfo(resource);

        auto createInfo = foeResourceGetCreateInfo(resource);
        REQUIRE(createInfo != FOE_NULL_HANDLE);

        CHECK(foeResourceCreateInfoGetRefCount(createInfo) == 2);
        CHECK(((TestData *)foeResourceCreateInfoGetData(createInfo))->value == 0);
        CHECK(gGlobalIterator == 1);

        SECTION("Check that importing again leads to different createInfo with newer data") {
            SECTION("Decrementing original leads to it being destroyed udring re-import") {
                CHECK(foeResourceCreateInfoDecrementRefCount(createInfo) == 1);
            }

            foeResourceImportCreateInfo(resource);

            auto createInfo2 = foeResourceGetCreateInfo(resource);
            REQUIRE(createInfo2 != FOE_NULL_HANDLE);
            CHECK(createInfo != createInfo2);

            CHECK(foeResourceCreateInfoGetRefCount(createInfo2) == 2);
            CHECK(((TestData *)foeResourceCreateInfoGetData(createInfo2))->value == 1);
            CHECK(gGlobalIterator == 2);

            CHECK(foeResourceCreateInfoDecrementRefCount(createInfo2) == 1);

            SECTION("Decrementing after re-import leads to an orphaned createInfo instead") {
                CHECK(foeResourceCreateInfoDecrementRefCount(createInfo) == 0);
                foeDestroyResourceCreateInfo(createInfo);
            }
        }
    }
    SECTION("Import asynchronously") {
        auto asyncRunFn = [](void *, PFN_foeTask task, void *pTaskContext) { task(pTaskContext); };

        gGlobalIterator = 0;
        fns.scheduleAsyncTask = asyncRunFn;

        CHECK_FALSE(foeResourceGetIsLoading(resource));
        foeResourceImportCreateInfo(resource);
        CHECK_FALSE(foeResourceGetIsLoading(resource));

        auto createInfo = foeResourceGetCreateInfo(resource);

        REQUIRE(createInfo != FOE_NULL_HANDLE);
        CHECK(((TestData *)foeResourceCreateInfoGetData(createInfo))->value == 0);
        CHECK(gGlobalIterator == 1);

        SECTION("Check that importing again leads to different createInfo with newer data") {
            SECTION("Decrementing original leads to it being destroyed udring re-import") {
                CHECK(foeResourceCreateInfoDecrementRefCount(createInfo) == 1);
            }

            foeResourceImportCreateInfo(resource);

            auto createInfo2 = foeResourceGetCreateInfo(resource);
            REQUIRE(createInfo2 != FOE_NULL_HANDLE);
            CHECK(createInfo != createInfo2);

            CHECK(foeResourceCreateInfoGetRefCount(createInfo2) == 2);
            CHECK(((TestData *)foeResourceCreateInfoGetData(createInfo2))->value == 1);
            CHECK(gGlobalIterator == 2);

            CHECK(foeResourceCreateInfoDecrementRefCount(createInfo2) == 1);

            SECTION("Decrementing after re-import leads to an orphaned createInfo instead") {
                CHECK(foeResourceCreateInfoDecrementRefCount(createInfo) == 0);
                foeDestroyResourceCreateInfo(createInfo);
            }
        }
    }

    foeDestroyResource(resource);
}