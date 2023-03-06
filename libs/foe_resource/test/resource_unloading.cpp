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

// Loading

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

auto loadDataFn = [](void *pSrc, void *pDst) { memcpy(pDst, pSrc, sizeof(TestResource)); };

// Unloading

struct DelayedUnloadingData {
    foeResource resource;
    uint32_t resourceIteration;
    PFN_foeResourceUnloadCall unloadCall;
};

auto unloadDataFn = [](void *pUnloadContext, void *pRaw) { memset(pRaw, 0, sizeof(TestResource)); };

bool processDelayedUnloading(DelayedUnloadingData *pDelayedUnloadingData) {
    return pDelayedUnloadingData->unloadCall(pDelayedUnloadingData->resource,
                                             pDelayedUnloadingData->resourceIteration, nullptr,
                                             unloadDataFn);
}

auto unloadFn = [](void *pUnloadContext,
                   foeResource resource,
                   uint32_t resourceIteration,
                   PFN_foeResourceUnloadCall unloadCall,
                   bool immediate) {
    if (immediate) {
        bool unloaded = unloadCall(resource, resourceIteration, nullptr, unloadDataFn);
    } else {
        DelayedUnloadingData *pDelayedUnloadingData = (DelayedUnloadingData *)pUnloadContext;
        *pDelayedUnloadingData = {
            .resource = resource,
            .resourceIteration = resourceIteration,
            .unloadCall = unloadCall,
        };
    }
};

} // namespace

TEST_CASE("foeResource - Unloading a 'unloaded' resource") {
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

    CHECK_FALSE(foeResourceGetIsLoading(resource));
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);

    // Performing an unload does nothing untoward, no state change
    SECTION("Immediate") { foeResourceUnloadData(resource, true); }
    SECTION("Delayed") { foeResourceUnloadData(resource, false); }

    CHECK_FALSE(foeResourceGetIsLoading(resource));
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);

    // Cleanup
    CHECK(foeResourceDecrementRefCount(resource) == 0);
}

TEST_CASE("foeResource - Unloading a 'failed' resource") {
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

    result = foeResourceLoadData(resource);
    REQUIRE(result.value == FOE_SUCCESS);

    // Post as a 'failed' load
    postLoadFn(resource, foeResultSet{.value = -1, .toString = errToString}, nullptr, nullptr,
               nullptr, nullptr);

    CHECK_FALSE(foeResourceGetIsLoading(resource));
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_FAILED);

    // Performing an unload does nothing untoward, no state change
    SECTION("Immediate") { foeResourceUnloadData(resource, true); }
    SECTION("Delayed") { foeResourceUnloadData(resource, false); }

    CHECK_FALSE(foeResourceGetIsLoading(resource));
    // Still considered a 'failed' resource
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_FAILED);

    // Cleanup
    CHECK(foeResourceDecrementRefCount(resource) == 0);
}

TEST_CASE("foeResource - Can not unload a replaced resource") {
    foeResource resource{FOE_NULL_HANDLE};
    foeResourceFns fns{};
    foeResultSet result;

    result = foeCreateUndefinedResource(0, &fns, &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // Initially there is no replacement resource, as this object is 'unloaded'
    CHECK(foeResourceGetReplacement(resource) == FOE_NULL_HANDLE);
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);

    // Create the resource to act as the 'replacement' (type does not matter)
    foeResource replacementResource{FOE_NULL_HANDLE};
    result = foeCreateUndefinedResource(0, &fns, &replacementResource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // The ref-count of the replacement resource should be 1, denoting the returned handle
    CHECK(foeResourceGetRefCount(replacementResource) == 1);

    result = foeResourceReplace(resource, replacementResource);
    REQUIRE(result.value == FOE_SUCCESS);

    // The old resource should now be considered as 'loaded', and the replacement resource
    // should be accurate
    CHECK(foeResourceGetReplacement(resource) == replacementResource);
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);
    CHECK(foeResourceDecrementRefCount(replacementResource) == 2);

    // Attempt to unload the replaced resource
    foeResourceUnloadData(resource, true);

    // No changes
    CHECK(foeResourceGetReplacement(resource) == replacementResource);
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);
    CHECK(foeResourceDecrementRefCount(replacementResource) == 2);

    // Cleanup
    CHECK(foeResourceDecrementRefCount(resource) == 0);
    CHECK(foeResourceDecrementRefCount(replacementResource) == 0);
}

TEST_CASE("foeResource - Unloading a regular resource") {
    // Loading
    PFN_foeResourcePostLoad postLoadFn = nullptr;
    foeResourceFns resourceFns{
        .pLoadContext = &postLoadFn,
        .pLoadFn = startLoadCall,
    };
    // Unloading
    DelayedUnloadingData delayedUnloadingData = {};
    // Other
    foeResource resource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResource(0, cTestResourceType, &resourceFns, sizeof(TestResource), &resource);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(resource != FOE_NULL_HANDLE);

    result = foeResourceLoadData(resource);
    REQUIRE(result.value == FOE_SUCCESS);

    SECTION("With no provided unload function") {
        postLoadFn(resource, foeResultSet{.value = FOE_SUCCESS}, &testData, loadDataFn, nullptr,
                   nullptr);
        REQUIRE(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);
        REQUIRE(memcmp(foeResourceGetData(resource), &testData, sizeof(TestResource)) == 0);

        SECTION("Immediate unloading happens immediately") {
            foeResourceUnloadData(resource, true);

            REQUIRE(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
        }
        SECTION("Delayed unloading happens immediately") {
            foeResourceUnloadData(resource, false);

            REQUIRE(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
        }
    }
    SECTION("With provided unload function") {
        postLoadFn(resource, foeResultSet{.value = FOE_SUCCESS}, &testData, loadDataFn,
                   &delayedUnloadingData, unloadFn);
        REQUIRE(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);
        REQUIRE(memcmp(foeResourceGetData(resource), &testData, sizeof(TestResource)) == 0);

        SECTION("Immediate Unloading") {
            SECTION("When use count is zero") {}
            SECTION("When use count is non-zero") { foeResourceIncrementUseCount(resource); }

            foeResourceUnloadData(resource, true);

            // State has been changed to 'unloaded'
            CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
            CHECK(foeResourceGetType(resource) == cTestResourceType);
            CHECK(foeResourceGetRefCount(resource) == 1);
        }
        SECTION("Delayed Unloading") {
            SECTION("When use count is zero") {}
            SECTION("When use count is non-zero") { foeResourceIncrementUseCount(resource); }

            foeResourceUnloadData(resource, false);

            // State is still 'loaded'
            CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);
            CHECK(foeResourceGetType(resource) == cTestResourceType);
            // Reference count incremented, captured by whatever has the resource handle for
            // unloading
            CHECK(foeResourceGetRefCount(resource) == 2);

            // Running the delayed unload call completes the unload as expected
            processDelayedUnloading(&delayedUnloadingData);

            CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
            CHECK(foeResourceGetType(resource) == cTestResourceType);
            CHECK(foeResourceGetRefCount(resource) == 1);
        }
    }

    CHECK(foeResourceDecrementRefCount(resource) == 0);
}