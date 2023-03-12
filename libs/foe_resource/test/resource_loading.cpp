// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/resource.h>
#include <foe/resource/resource_fns.h>
#include <foe/resource/result.h>
#include <foe/resource/type_defs.h>

#include <cstring>

namespace {

constexpr foeResourceType cTestResourceType = 0xf0f0f0;

struct TestResource {
    foeResourceType rType;
    void *pNext;
    int value1;
    int value2;
    char const *pStr;
};

void errToString(int value, char buffer[FOE_MAX_RESULT_STRING_SIZE]) {
    memcpy(buffer, "TestError", sizeof("TestError"));
}

void startLoadCall(void *pContext, foeResource resource, PFN_foeResourcePostLoad postLoadFn) {
    PFN_foeResourcePostLoad *pPostLoadFn = (PFN_foeResourcePostLoad *)pContext;

    *pPostLoadFn = postLoadFn;
}

TestResource testData = {
    .rType = cTestResourceType,
    .value1 = 1,
    .value2 = 9090,
    .pStr = "Hello World!",
};

TestResource testData2 = {
    .rType = cTestResourceType,
    .value1 = 1500,
    .value2 = 1980,
    .pStr = "Hello World!2",
};

auto loadDataFn = [](void *pSrc, void *pDst) { memcpy(pDst, pSrc, sizeof(TestResource)); };

} // namespace

TEST_CASE("foeResource - Replaced resource can not be loaded (use replacement instead)") {
    foeResourceFns resourceFns{
        .pLoadContext = NULL,
        .pLoadFn = startLoadCall,
    };
    foeResource resource{FOE_NULL_HANDLE};
    foeResource replacementResource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateUndefinedResource(0, &resourceFns, &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    result = foeCreateUndefinedResource(0, &resourceFns, &replacementResource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(replacementResource != FOE_NULL_HANDLE);

    result = foeResourceReplace(resource, replacementResource);
    REQUIRE(result.value == FOE_SUCCESS);

    result = foeResourceLoadData(resource);
    CHECK(result.value == FOE_RESOURCE_ERROR_REPLACED_CANNOT_BE_LOADED);

    // Cleanup
    REQUIRE(foeResourceDecrementRefCount(resource) == 0);
    REQUIRE(foeResourceDecrementRefCount(replacementResource) == 0);
}

TEST_CASE("foeResource - Loading via provided resource functions") {
    PFN_foeResourcePostLoad postLoadFn = nullptr;
    foeResourceFns resourceFns{
        .pLoadContext = &postLoadFn,
        .pLoadFn = startLoadCall,
    };
    foeResource resource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResource(0, cTestResourceType, &resourceFns, sizeof(TestResource), &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    CHECK(foeResourceGetRefCount(resource) == 1);
    CHECK(foeResourceGetState(resource) == (foeResourceStateFlags)0);

    result = foeResourceLoadData(resource);
    REQUIRE(result.value == FOE_SUCCESS);

    // When a load call is successful, then reference count should have been incremented
    // (Prevents resource from destruction when still in use by the loading process)
    CHECK(foeResourceGetRefCount(resource) == 2);
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_STATE_LOADING_BIT);

    // Check the postLoadFn was set as expected (by `startLoadCall`)
    REQUIRE(postLoadFn != nullptr);

    SECTION("Attempting to load while being loaded by another results in ALREADY_LOADING qualified "
            "success code") {
        result = foeResourceLoadData(resource);
        CHECK(result.value == FOE_RESOURCE_ALREADY_LOADING);

        // Does not increase reference count
        CHECK(foeResourceGetRefCount(resource) == 2);

        // Cleanup
        CHECK(foeResourceDecrementRefCount(resource) == 1);
    }

    // After loading is finished, postLoadFn is called with appropriate parameters
    SECTION("Giving an error code will leave the resource with the FAILED load state") {
        postLoadFn(resource, foeResultSet{.value = -1, .toString = errToString}, nullptr, nullptr,
                   nullptr, nullptr);

        CHECK(foeResourceGetState(resource) == FOE_RESOURCE_STATE_FAILED_BIT);
        CHECK(foeResourceGetRefCount(resource) == 1);

        SECTION("Subsequent successful load leaves resource in LOADED state") {
            result = foeResourceLoadData(resource);
            REQUIRE(result.value == FOE_SUCCESS);

            CHECK(foeResourceGetState(resource) ==
                  (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_FAILED_BIT));

            postLoadFn(resource, foeResultSet{.value = FOE_SUCCESS}, &testData2, loadDataFn,
                       nullptr, nullptr);

            CHECK(foeResourceGetState(resource) == FOE_RESOURCE_STATE_LOADED_BIT);

            void const *pResourceData = foeResourceGetData(resource);
            CHECK(pResourceData != nullptr);
            CHECK(pResourceData == foeResourceGetTypeData(resource, cTestResourceType));
            CHECK(memcmp(&testData, pResourceData, sizeof(TestResource)) != 0);
            CHECK(memcmp(&testData2, pResourceData, sizeof(TestResource)) == 0);
        }
    }
    SECTION("Successful loading will leave the resource with the LOADED load state") {
        postLoadFn(resource, foeResultSet{.value = FOE_SUCCESS}, &testData, loadDataFn, nullptr,
                   nullptr);

        CHECK(foeResourceGetState(resource) == FOE_RESOURCE_STATE_LOADED_BIT);
        CHECK(foeResourceGetRefCount(resource) == 1);

        // Check that the data can be retrieved, and matches what was given
        void const *pResourceData = foeResourceGetData(resource);
        CHECK(pResourceData != nullptr);
        CHECK(pResourceData == foeResourceGetTypeData(resource, cTestResourceType));
        CHECK(memcmp(&testData, pResourceData, sizeof(TestResource)) == 0);

        SECTION("Performing a subsequent successful load") {
            result = foeResourceLoadData(resource);
            REQUIRE(result.value == FOE_SUCCESS);

            CHECK(foeResourceGetRefCount(resource) == 2);
            CHECK(foeResourceGetState(resource) ==
                  (FOE_RESOURCE_STATE_LOADING_BIT | FOE_RESOURCE_STATE_LOADED_BIT));

            postLoadFn(resource, foeResultSet{.value = FOE_SUCCESS}, &testData2, loadDataFn,
                       nullptr, nullptr);

            void const *pResourceData = foeResourceGetData(resource);
            CHECK(pResourceData != nullptr);
            CHECK(pResourceData == foeResourceGetTypeData(resource, cTestResourceType));
            CHECK(memcmp(&testData, pResourceData, sizeof(TestResource)) != 0);
            CHECK(memcmp(&testData2, pResourceData, sizeof(TestResource)) == 0);
        }
    }

    // Cleanup
    REQUIRE(foeResourceDecrementRefCount(resource) == 0);
}

TEST_CASE("foeResource - Loading via async task function") {
    bool asyncTaskCalled = false;
    auto asyncTaskFn = [](void *pAsyncContext, PFN_foeTask taskFn, void *pTaskContext) {
        bool *pAsyncTaskCalled = (bool *)pAsyncContext;
        *pAsyncTaskCalled = true;

        taskFn(pTaskContext);
    };

    PFN_foeResourcePostLoad postLoadFn = nullptr;
    foeResourceFns resourceFns{
        .pLoadContext = &postLoadFn,
        .pLoadFn = startLoadCall,
        .scheduleAsyncTask = asyncTaskFn,
        .pScheduleAsyncTaskContext = &asyncTaskCalled,
    };
    foeResource resource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResource(0, cTestResourceType, &resourceFns, sizeof(TestResource), &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    REQUIRE_FALSE(asyncTaskCalled);
    result = foeResourceLoadData(resource);
    REQUIRE(result.value == FOE_SUCCESS);

    // Make sure async task was called instead of being done via synchronous path
    REQUIRE(asyncTaskCalled);

    postLoadFn(resource, foeResultSet{.value = -1, .toString = errToString}, nullptr, nullptr,
               nullptr, nullptr);

    // Cleanup
    REQUIRE(foeResourceDecrementRefCount(resource) == 0);
}