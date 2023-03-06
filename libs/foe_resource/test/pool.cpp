// Copyright (C) 2023 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <catch.hpp>
#include <foe/resource/pool.h>
#include <foe/resource/resource_fns.h>
#include <foe/resource/result.h>

#include <cstring>

namespace {

constexpr foeResourceType cTestResourceType = 0xf0f0f0;
constexpr foeResourceType cTestResourceType2 = 0x0f0f0f;

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

TestResource testData2 = {
    .rType = cTestResourceType2,
    .value1 = 1,
    .value2 = 9090,
    .pStr = "Hello World!",
};

auto loadDataFn = [](void *pSrc, void *pDst) { memcpy(pDst, pSrc, sizeof(TestResource)); };

// Unloading

struct UnloadedData {
    foeResource resource;
    foeResourceType type;
    bool immediate;
};

auto unloadDataFn = [](void *pUnloadContext, void *pRaw) { memset(pRaw, 0, sizeof(TestResource)); };

auto unloadFn = [](void *pUnloadContext,
                   foeResource resource,
                   uint32_t resourceIteration,
                   PFN_foeResourceUnloadCall unloadCall,
                   bool immediate) {
    UnloadedData *pUnloadedData = (UnloadedData *)pUnloadContext;
    *pUnloadedData = {
        .resource = resource,
        .type = foeResourceGetType(resource),
        .immediate = immediate,
    };

    unloadCall(resource, resourceIteration, nullptr, unloadDataFn);
};

} // namespace

TEST_CASE("foeResourcePool - Basic pool create and destroy") {
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    SECTION("Attempting to remove a resource not in the pool returns an error") {
        result = foeResourcePoolRemove(pool, 0);
        REQUIRE(result.value == FOE_RESOURCE_ERROR_NOT_FOUND);
    }

    foeDestroyResourcePool(pool);
}

TEST_CASE("foeResourcePool - Add/remove of an undefined resource to the pool") {
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource resource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    resource = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // Type is UNDEFINED
    CHECK(foeResourceGetType(resource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED);
    // The resource ref count is 2, one for the returned reference, one held by the pool
    CHECK(foeResourceGetRefCount(resource) == 2);

    SECTION("Removing the resource from the pool decrements the reference count") {
        result = foeResourcePoolRemove(pool, 0);
        REQUIRE(result.value == FOE_SUCCESS);

        CHECK(foeResourceGetRefCount(resource) == 1);
        CHECK(foeResourceDecrementRefCount(resource) == 0);
    }

    // Destroying the pool
    foeDestroyResourcePool(pool);

    SECTION("If the reference count was not decremented before pool destruction, the pool's "
            "destruction decrements it by one") {
        // After destroying the pool, the resource ref count is down to just 1
        CHECK(foeResourceGetRefCount(resource) == 1);
        CHECK(foeResourceDecrementRefCount(resource) == 0);
    }
}

TEST_CASE("foeResourcePool - Cannot add the same ID again until after it is first removed") {
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource resource{FOE_NULL_HANDLE}, resource2{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    resource = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource != FOE_NULL_HANDLE);
    CHECK(foeResourceDecrementRefCount(resource) == 1);

    // Cannot add the same ID again
    resource2 = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource2 == FOE_NULL_HANDLE);

    // Removing then re-adding can work
    foeResourcePoolRemove(pool, 0);
    resource2 = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource2 != FOE_NULL_HANDLE);
    CHECK(foeResourceDecrementRefCount(resource2) == 1);

    foeDestroyResourcePool(pool);
}

TEST_CASE("foeResourcePool - Replacing a resource in a pool") {
    UnloadedData unloadedData = {};
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource resource{FOE_NULL_HANDLE};
    foeResource replacementResource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    resource = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource != FOE_NULL_HANDLE);

    // Original resource has two references (pool and returned handle)
    CHECK(foeResourceGetRefCount(resource));
    // Original resource is undefined, unloaded, no available replacement
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
    CHECK(foeResourceGetType(resource) == FOE_RESOURCE_RESOURCE_TYPE_UNDEFINED);
    CHECK(foeResourceGetReplacement(resource) == FOE_NULL_HANDLE);

    replacementResource =
        foeResourcePoolLoadedReplace(pool, 0, cTestResourceType, sizeof(TestResource), &testData,
                                     loadDataFn, &unloadedData, unloadFn);
    REQUIRE(replacementResource != FOE_NULL_HANDLE);

    // Old resource indicates that a replacement has happened and points to the same new resource
    CHECK(foeResourceGetState(resource) == FOE_RESOURCE_LOAD_STATE_LOADED);
    CHECK(foeResourceGetType(resource) == FOE_RESOURCE_RESOURCE_TYPE_REPLACED);
    // Old resource ref count decremented by one (pool ref removed)
    CHECK(foeResourceGetRefCount(resource) == 1);
    // Replacement resource has three references (pool, replacement and returned handles)
    CHECK(foeResourceGetRefCount(replacementResource) == 3);

    // Getting the replacement from the original resource is corrent and increments ref count
    CHECK(foeResourceGetReplacement(resource) == replacementResource);
    CHECK(foeResourceGetRefCount(replacementResource) == 4);
    CHECK(foeResourceDecrementRefCount(replacementResource) == 3);

    // Destroying the original resource decrements the replacement ref count
    CHECK(foeResourceDecrementRefCount(resource) == 0);
    CHECK(foeResourceGetRefCount(replacementResource) == 2);

    // Destroying the pool properly decrements the replacement resources ref count
    foeDestroyResourcePool(pool);
    CHECK(foeResourceGetRefCount(replacementResource) == 1);

    // Nothing has been unloaded yet
    CHECK(unloadedData.resource == FOE_NULL_HANDLE);
    // Decrement the last resource
    CHECK(foeResourceDecrementRefCount(replacementResource) == 0);
    // It was now unloaded
    CHECK(unloadedData.resource == replacementResource);
    CHECK(unloadedData.type == cTestResourceType);
    CHECK(unloadedData.immediate);
}

TEST_CASE("foeResourcePool - Resource replacement failures") {
    UnloadedData unloadedData = {};
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource replacementResource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    SECTION("Resource to replace does not exist in the pool") {
        replacementResource =
            foeResourcePoolLoadedReplace(pool, 0, cTestResourceType, sizeof(TestResource),
                                         &testData, loadDataFn, &unloadedData, unloadFn);
        REQUIRE(replacementResource == FOE_NULL_HANDLE);
    }
    SECTION("Resource was already replaced") {
        foeResource resource{FOE_NULL_HANDLE};

        resource = foeResourcePoolAdd(pool, 0);
        REQUIRE(resource != FOE_NULL_HANDLE);

        replacementResource =
            foeResourcePoolLoadedReplace(pool, 0, cTestResourceType, sizeof(TestResource),
                                         &testData, loadDataFn, &unloadedData, unloadFn);
        REQUIRE(replacementResource != FOE_NULL_HANDLE);

        foeResource failResourceReplacement{FOE_NULL_HANDLE};
        failResourceReplacement =
            foeResourcePoolLoadedReplace(pool, 0, cTestResourceType, sizeof(TestResource),
                                         &testData, loadDataFn, &unloadedData, unloadFn);
        CHECK(failResourceReplacement == FOE_NULL_HANDLE);

        // Cleanup the original undefined resource handle
        CHECK(foeResourceDecrementRefCount(resource) == 0);
        // Cleanup the replacement resource handle
        CHECK(foeResourceDecrementRefCount(replacementResource) == 1);
    }

    foeDestroyResourcePool(pool);
}

TEST_CASE("foeResourcePool - Finding resource in a pool") {
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource resource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    SECTION("Before being added to a pool, a search returns FOE_NULL_HANDLE") {
        resource = foeResourcePoolFind(pool, 0);
        CHECK(resource == FOE_NULL_HANDLE);
    }

    resource = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource != FOE_NULL_HANDLE);

    CHECK(foeResourceGetRefCount(resource) == 2);

    SECTION("After being added to a pool, a search returns the handle, with the reference count "
            "incremented") {
        foeResource foundResource = foeResourcePoolFind(pool, 0);
        REQUIRE(foundResource != FOE_NULL_HANDLE);

        CHECK(foundResource == resource);

        CHECK(foeResourceGetRefCount(foundResource) == 3);
        CHECK(foeResourceDecrementRefCount(foundResource) == 2);
    }

    CHECK(foeResourceDecrementRefCount(resource) == 1);
    // Destroying the pool
    foeDestroyResourcePool(pool);
}

TEST_CASE("foeResourcePool - Unloading resources via pool") {
    UnloadedData unloaded1{}, unloaded2{};
    foeResourceFns resourceFns{};
    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource resource1{FOE_NULL_HANDLE}, resource2{FOE_NULL_HANDLE};
    foeResource replacement1{FOE_NULL_HANDLE}, replacement2{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    // Resource  Type 1
    resource1 = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource1 != FOE_NULL_HANDLE);
    CHECK(foeResourceDecrementRefCount(resource1) == 1);

    replacement1 = foeResourcePoolLoadedReplace(pool, 0, cTestResourceType, sizeof(TestResource),
                                                &testData, loadDataFn, &unloaded1, unloadFn);
    REQUIRE(replacement1 != FOE_NULL_HANDLE);
    CHECK(foeResourceGetState(replacement1) == FOE_RESOURCE_LOAD_STATE_LOADED);
    CHECK(foeResourceDecrementRefCount(replacement1) == 1);

    // Resource Type 2
    resource2 = foeResourcePoolAdd(pool, 1);
    REQUIRE(resource2 != FOE_NULL_HANDLE);
    CHECK(foeResourceDecrementRefCount(resource2) == 1);

    replacement2 = foeResourcePoolLoadedReplace(pool, 1, cTestResourceType2, sizeof(TestResource),
                                                &testData2, loadDataFn, &unloaded2, unloadFn);
    REQUIRE(replacement2 != FOE_NULL_HANDLE);
    CHECK(foeResourceGetState(replacement2) == FOE_RESOURCE_LOAD_STATE_LOADED);
    CHECK(foeResourceDecrementRefCount(replacement2) == 1);

    SECTION("Unloading per-type") {
        SECTION("Type 1") {
            CHECK(foeResourcePoolUnloadType(pool, cTestResourceType) == 1);

            // Type 1 was unloaded
            CHECK(foeResourceGetState(replacement1) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
            CHECK(unloaded1.resource == replacement1);
            CHECK(unloaded1.type == cTestResourceType);
            CHECK_FALSE(unloaded1.immediate);

            // Type 2 is unchanged
            CHECK(foeResourceGetState(replacement2) == FOE_RESOURCE_LOAD_STATE_LOADED);
            CHECK(unloaded2.resource == FOE_NULL_HANDLE);
        }
        SECTION("Type 2") {
            CHECK(foeResourcePoolUnloadType(pool, cTestResourceType2) == 1);

            // Type 2 was unloaded
            CHECK(foeResourceGetState(replacement2) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
            CHECK(unloaded2.resource == replacement2);
            CHECK(unloaded2.type == cTestResourceType2);
            CHECK_FALSE(unloaded2.immediate);

            // Type 1 is unchanged
            CHECK(foeResourceGetState(replacement1) == FOE_RESOURCE_LOAD_STATE_LOADED);
            CHECK(unloaded1.resource == FOE_NULL_HANDLE);
        }
    }
    SECTION("Unloading all") {
        foeResourcePoolUnloadAll(pool);

        // Type 1 was unloaded
        CHECK(foeResourceGetState(replacement1) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
        CHECK(unloaded1.resource == replacement1);
        CHECK(unloaded1.type == cTestResourceType);
        CHECK_FALSE(unloaded1.immediate);

        // Type 2 was unloaded
        CHECK(foeResourceGetState(replacement2) == FOE_RESOURCE_LOAD_STATE_UNLOADED);
        CHECK(unloaded2.resource == replacement2);
        CHECK(unloaded2.type == cTestResourceType2);
        CHECK_FALSE(unloaded2.immediate);
    }

    foeDestroyResourcePool(pool);
}

TEST_CASE("foeResourcePool - Check adding async to pool propagates to resources") {
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
    };

    foeResourcePool pool{FOE_NULL_HANDLE};
    foeResource resource{FOE_NULL_HANDLE};
    foeResultSet result;

    result = foeCreateResourcePool(&resourceFns, &pool);
    REQUIRE(result.value == FOE_SUCCESS);
    REQUIRE(pool != FOE_NULL_HANDLE);

    resource = foeResourcePoolAdd(pool, 0);
    REQUIRE(resource != FOE_NULL_HANDLE);

    SECTION("Original synchronized loading") {
        result = foeResourceLoadData(resource);
        CHECK(result.value == FOE_SUCCESS);

        CHECK_FALSE(asyncTaskCalled);
    }
    SECTION("With async tasks added") {
        foeResourcePoolSetAsyncTaskCallback(pool, &asyncTaskCalled, asyncTaskFn);

        result = foeResourceLoadData(resource);
        CHECK(result.value == FOE_SUCCESS);

        CHECK(asyncTaskCalled);
    }

    postLoadFn(resource, foeResultSet{.value = -1, .toString = errToString}, nullptr, nullptr,
               nullptr, nullptr);

    // Cleanup
    CHECK(foeResourceDecrementRefCount(resource) == 1);
    foeDestroyResourcePool(pool);
}